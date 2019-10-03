#include "rtc_base/timeutils.h"
#include "endecoder/videocodec.h"
#include "record/yuv_record.h"
#include <iostream>
#include <fstream>
#include <string>
namespace zsy{
EncodeImage::EncodeImage(const uint8_t*data,size_t len,uint32_t ft,
            uint32_t capture_ts,uint32_t encode_ts){
    buffer_=new RefCountedObject<Buffer>();
    ft_=ft;
    capture_ts_=capture_ts;
    encode_ts_=encode_ts;
    buffer_->SetSize(len);
    buffer_->SetData(data,len);
}
const uint8_t *EncodeImage::data() const{
    const uint8_t *ret=nullptr;
    if(buffer_){
        ret=buffer_->data();
    }
    return ret;
}
uint8_t *EncodeImage::data(){
    uint8_t *ret=nullptr;
    if(buffer_){
        ret=buffer_->data();
    }
    return ret;
}
VideoEncoder::VideoEncoder(int width,int height,int fps)
:width_(width),height_(height),fps_(fps){
	yuv_buf_=new RefCountedObject<Buffer>(width*height*3/2);
	encoder_.reset(new H264Encoder());
}
VideoEncoder::~VideoEncoder(){
    if(running_){
    running_=false;
    /*rtc::Thread::*/Stop();
    }
    LockScope ls(&lock_);
    while(!frames_.empty()){
    	frames_.pop_front();
    }
    encoder_.reset(nullptr);
}
void VideoEncoder::StartEncoder(){
	bool flag=false;
	flag=encoder_->init(fps_,width_,height_,width_,height_);
	if(!flag){
		perror("init encoder error");
		abort();
	}
	if(!running_){
		running_=true;
		/*rtc::Thread::*/Start();
	}
}
void VideoEncoder::StopEncoder(){
    if(running_){
    running_=false;
    /*rtc::Thread::*/Stop();
    }
}
void VideoEncoder::Run(){
	while(running_){
		webrtc::VideoFrame*f=nullptr;
		uint32_t enque_ts=0;
		if(que_len_>0)
		{
			LockScope ls(&lock_);
			if(!frames_.empty()){
				FrameTs temp=frames_.front();
				f=temp.frame;
				temp.frame=nullptr;
				enque_ts=temp.enqueTs;
				frames_.pop_front();
				que_len_--;
			}
		}
		if(f){
			int width=f->width();
			int height=f->height();
			int size = width*height * 3 / 2;
			yuv_buf_->SetSize(size);
			
			webrtc::ConvertFromI420(*f, webrtc::kI420, 0, yuv_buf_->data());
			int out_size=0;
			int ft=0;
			uint32_t last=GetMilliSeconds();
            bool ret=false;
			ret=encoder_->encode(yuv_buf_->data(),size
					,AV_PIX_FMT_YUV420P,image_buf_,
					&out_size,&ft,false);
			uint32_t now=GetMilliSeconds();
			uint32_t capture_ts=f->timestamp_us()/1000;
			uint32_t queue=last-enque_ts;
			pic_id_++;
			std::cout<<total_pid_<<" "<<pic_id_<<" "<<out_size<<" "<<
			queue<<" "<<que_len_<<std::endl;
            if(ret){
            	if(!sinks_.empty()){
            		EncodeImage image(image_buf_,out_size,ft,capture_ts,now);
                    for(auto it=sinks_.begin();it!=sinks_.end();it++){
                        EncodedVideoCallback *cb=(*it);
                        cb->OnEncodedImageCallBack(image);
                    }
            	}
            }else{
                std::cout<<"encode failed"<<std::endl;
            }

			delete f;
		}
	}
}
void VideoEncoder::OnFrame(const webrtc::VideoFrame& frame){
	 total_pid_++;
	 //if(que_len_>max_que_){return ;}
     // the two time stamp  values are same
     //uint32_t capture_ts=frame.timestamp_us()/1000;
     //std::cout<<frame.render_time_ms()<<" "<<capture_ts<<std::endl;
	 webrtc::VideoFrame *copy=new webrtc::VideoFrame(frame);
	 uint32_t now=GetMilliSeconds();
	 LockScope ls(&lock_);
     if(!frames_.empty()){
     FrameTs temp=frames_.front();
     uint32_t queue_delay=now-temp.enqueTs;
     if(queue_delay>max_que_delay_){
         return ;
     }}
     if(yuv_){
    	 rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer=
    	 				 frame.video_frame_buffer()->ToI420();

    	 yuv_->OnFrame(frame);
     }
	 frames_.push_back(FrameTs(copy,now));
	 que_len_++;
}
bool VideoEncoder::RegisterSink(EncodedVideoCallback *cb){
    bool success=false;
    bool exist=false;
    for(auto it=sinks_.begin();it!=sinks_.end();it++){
        if(cb==(*it)){
            exist=true;
        }
    }
    if(!exist){
        sinks_.push_back(cb); 
    }
    return success;
}
void VideoEncoder::RegisterYUVRecord(rtc::VideoSinkInterface<webrtc::VideoFrame> *yuv){
    yuv_=yuv;
}
void VideoEncoder::SetRate(uint32_t r){
	encoder_->set_bitrate(r);
}
VideoDecoder::VideoDecoder(uint32_t h,uint32_t w){
    height_=h;
    width_=w;
    uint32_t len=width_*height_*3;
//TODO, not quite safe when resolution changes!
    yuv_buf_=new RefCountedObject<Buffer>(len);
    handler_=decoder_.X264Decoder_Init();
}
VideoDecoder::~VideoDecoder(){
    StopDecoder();
}
void VideoDecoder::StartDecoder(){
    if(!running_){
        running_=true;
        Start();
    }
}
void VideoDecoder::StopDecoder(){
    if(running_){
        running_=false;
        Stop();
        decoder_.X264Decoder_UnInit(handler_);
    }
}
void VideoDecoder::Run(){
    while(running_){
        std::list<EncodeImage> temp;
        if(que_len_>0){
            LockScope ls(&que_lock_);
            while(!images_.empty()){
                EncodeImage image=images_.front();
                images_.pop_front();
                que_len_--;
                temp.push_back(image);
            }
        }
        while(!temp.empty()){
            EncodeImage image=temp.front();
            temp.pop_front();
            int decode_size=0;
            int h=0;
            int w=0;
            int ret=0;
            ret=decoder_.X264Decoder_Decode(handler_,image.data(),image.size(),yuv_buf_->data()
					,&decode_size,&w,&h);
            if(ret!=0){
                std::cout<<"decode error"<<std::endl;
            }else{  
                std::string name=std::to_string(decode_id_)+"_decode.yuv";
                if(record_id_<max_record_){
		    std::cout<<"decode image "<<" "<<h<<" "<<w<<std::endl;
                    std::ofstream out;
                    out.open(name.c_str(),std::ofstream::binary);
                    out.write((const char*)yuv_buf_->data(),decode_size);
                    out.close();                    
                }
                record_id_++;
            }
            if(sink_){
            	sink_->OnFrameAfterDecode(decode_id_,w,h,yuv_buf_->data(),decode_size);
            }
            decode_id_++; 
        }
    }
}
void VideoDecoder::ResetHxW(uint32_t height,uint32_t width){
    uint32_t len=height*width*3;
    if(yuv_buf_->size()<len){
    	yuv_buf_->SetSize(len);
    }
}
void VideoDecoder::OnEncodedImageCallBack(EncodeImage &image){
    image_count_++;
    //test impact of lost p frame
    if(image_count_==3||image_count_==4||image_count_==5||image_count_==6||image_count_==7||image_count_==8){
        return ;
    }
    LockScope ls(&que_lock_);
    que_len_++;
    //do not use move;
    images_.push_back(image);
}

}
