#include <iostream>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <fstream>
#include <list>
#include <vector>
#include "packetizer/h264_common.h"
#include "h264parser/h264_parser.h"
#include "endecoder/H264Decoder.h"
#include <assert.h>
#include "logging.h"
#include <utility>
#include <memory>
using namespace zsy;
using namespace std;
const uint8_t kTypeMask=0x1F;
int dropTable[]={4,5,6,7};
char startCode3[]={0x00,0x00,0x01};
char startCode4[]={0x00,0x00,0x00,0x01};
// error comes from sizeof(dropTable)/sizeof(int)
//  this cosr me nearly two hours;
bool shoudDrop(int f,int *dropframe,size_t size){
    bool ret=false;
    int i=0;
    for(i=0;i<size;i++){
        if(f==dropframe[i]){
            ret=true;
            break;
        }
    }
    return ret;
}
int calculateFrameSize(std::vector<H264NALU> &nalus){
    int size=0;
    int i=0;
    for(i=0;i<nalus.size();i++){
        H264NALU item=nalus[i];
        size+=(item.start_code_size+item.size);
    }
    return size;
}
int main(){
    char *h264_in="1280x720.h264";
    int fd=0;
    if((fd=open(h264_in,O_RDONLY,S_IRGRP))<0){
        printf("open failed \n");
        return 0;
    }
    struct stat file_stat;
    if((fstat(fd,&file_stat))<0){
        printf("fstat failed \n");
        return 0;        
    }
    void *start_fp;
    if((start_fp=mmap(NULL,file_stat.st_size,PROT_READ,MAP_SHARED,fd,0))==MAP_FAILED){
        printf("mmap failed \n");
        return 0;         
    }
    int length=file_stat.st_size;
    bool droped=false;
    int droped_frame_count=0;
    int height=1280;
    int width=720;
    H264Decoder decoder;
    X264_DECODER_H handler=decoder.X264Decoder_Init();  
    int yuv_buffer_len=height*width*3/2;
    std::unique_ptr<char[]> encoded_image;
    char yuv_420_buffer[yuv_buffer_len]={0};
    H264Parser parser;
    parser.SetStream((uint8_t*)start_fp,length);
    int i=0;
    int total_to_parse=100;
    H264NALU nalu;
    int frame_count=0;
    std::vector<H264NALU> nalus;
    std::fstream f_out;
    std::string out("out.h264");
    f_out.open(out.c_str(),ios::out|ios::binary);
    while(frame_count<total_to_parse){
        if(parser.AdvanceToNextNALU(&nalu)==H264Parser::kOk){
            uint8_t type=nalu.data[0]&kTypeMask;
	
        if((type==1)&&(nalu.start_code_size==4)){           
            if(!shoudDrop(frame_count,dropTable,sizeof(dropTable)/sizeof(int))){
            
                int encode_image_size=calculateFrameSize(nalus);
                encoded_image.reset(new char[encode_image_size]);
                int offset=0;
                bool copyed=true;
                for(i=0;i<nalus.size();i++){
                    H264NALU item=nalus[i];
                    if(item.start_code_size==4){
                        memcpy(encoded_image.get()+offset,startCode4,sizeof(startCode4));
                        offset+=sizeof(startCode4);
                    }else if(item.start_code_size==3){
                        memcpy(encoded_image.get()+offset,startCode3,sizeof(startCode3));
                        offset+=sizeof(startCode3);                    
                    }else{
                        copyed=false;
                        break;
                    }
                    memcpy(encoded_image.get()+offset,item.data,item.size);
                    offset+=item.size;
                }
                int decode_ret=0;
                if(copyed){
                    int w,h;
		    int out;
                    int decode_ret=decoder.X264Decoder_Decode(handler,(uint8_t*)encoded_image.get(),offset,(uint8_t*)yuv_420_buffer,&out,
                    &w,&h);
                }

                
                for(i=0;i<nalus.size();i++){
                    H264NALU item=nalus[i];
                    if(item.start_code_size==4){
                        f_out.write(startCode4,sizeof(startCode4));
                    }else if(item.start_code_size==3){
                        f_out.write(startCode3,sizeof(startCode3));
                    }else{
                        break;
                    }
                    f_out.write((const char*)item.data,item.size);
                }
                if(droped){
                    std::string name=std::string("drop_")+std::to_string(droped_frame_count)+std::string(".yuv");
                    FILE *fp=nullptr;
                    fp=fopen(name.c_str(), "wb");
                    if(fp&&(decode_ret==0)){
                        if(!fwrite(yuv_420_buffer,yuv_buffer_len,1,fp)){
                            std::cout<<"write error";
                        }
                    }
                    if(fp){
                        fclose(fp);
                    }
                    droped=false;
                }
            }else{
                droped=true;
                droped_frame_count++;
            }
            std::vector<H264NALU> nullVec;
            nalus.swap(nullVec);
            frame_count++;
          }
          nalus.push_back(nalu);
        }else{
            break;
        }
    }
    f_out.close();
    decoder.X264Decoder_UnInit(handler);
    return 0;
}
