#include <iostream>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

#include <list>
#include <vector>
#include "packetizer/h264_common.h"
#include "h264parser/h264_parser.h"
#include "logging.h"

using namespace zsy;
using namespace std;
const uint8_t kTypeMask=0x1F;
class Frame{
public:
	Frame(){}
	void NewNalu(H264NALU &nalu){
		nalus_.push_back(nalu);
	}
	size_t size() const{
		return nalus_.size();
	}
	uint8_t FrameType() const{
		uint8_t type=H264::kSlice;
		for(auto it=nalus_.begin();it!=nalus_.end();it++){
			if((it->data[0]&kTypeMask)==H264::kIdr){
				type=H264::kIdr;
				break;
			}
		}
		return type;
	}
private:
	std::list<H264NALU> nalus_;
};
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
    std::cout<<"size "<<length<<std::endl;
    H264Parser parser;
    parser.SetStream((uint8_t*)start_fp,length);
    int f=0;
    int total_to_parse=10;
    H264NALU nalu;
    std::list<Frame*> frames;
    int frame_count=0;
    Frame *frame=new Frame;
    frames.push_back(frame);
    while(f<total_to_parse){
        if(parser.AdvanceToNextNALU(&nalu)==H264Parser::kOk){
            uint8_t type=nalu.data[0]&kTypeMask;
        if((type==1)&&(nalu.start_code_size==4)){
        	frame=new Frame;
        	frames.push_back(frame);
          }
        if(frame_count<total_to_parse){
        	frame->NewNalu(nalu);
        }
        }else{
            break;
        }
        f++;
    }
    frame=frames.front();
    int nalus=frame->size();
    int frame_type=frame->FrameType();
    DLOG(INFO)<<"first frame "<<nalus<<" type "<<frame_type;
    while(!frames.empty()){
	frame=frames.front();
	frames.pop_front();
	delete frame;
    }
    return 0;
}
