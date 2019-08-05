#include "rtc_base/timeutils.h"
#include "videocodec.h"
#include <iostream>
namespace zsy{
VideoEncoder::VideoEncoder(int width,int height,int fps)
:width_(width),height_(height),fps_(fps){
	encoder_.reset(new H264Encoder());
}
VideoEncoder::~VideoEncoder(){
    if(running_){
    running_=false;
    rtc::Thread::Stop();
    }
    rtc::CritScope crit(&que_lock_);
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
		rtc::Thread::Start();
	}
}
void VideoEncoder::StopEncoder(){
    if(running_){
    running_=false;
    rtc::Thread::Stop();
    }
}
void VideoEncoder::Run(){
	while(running_){
		webrtc::VideoFrame*f=nullptr;
		{
			rtc::CritScope crit(&que_lock_);
			if(!frames_.empty()){
				f=new webrtc::VideoFrame(frames_.front());
				frames_.pop_front();
			}
		}
		if(f){
			int width=f->width();
			int height=f->height();
			std::unique_ptr<uint8_t >videoImage;
			int size = width*height * 3 / 2;
			videoImage.reset(new uint8_t[size]);
			webrtc::ConvertFromI420(*f, webrtc::kI420, 0, videoImage.get());
			int out_size=0;
			int ft=0;
			encoder_->encode((uint8_t*)videoImage.get(),size
					,AV_PIX_FMT_YUV420P,image_buf_,
					&out_size,&ft,false);
			uint32_t now=rtc::TimeMillis();
			uint32_t delta=now-f->timestamp_us()/1000;
			pic_id_++;
			std::cout<<"encode "<<pic_id_<<" "<<out_size<<std::endl;
			if(sink_){
				sink_->OnEncodedImageCallBack(image_buf_,out_size,ft,delta);
			}
			delete f;
		}
	}
}
void VideoEncoder::OnFrame(const webrtc::VideoFrame& frame){
	 webrtc::VideoFrame copy=frame;
	 rtc::CritScope crit(&que_lock_);
	 frames_.push_back(copy);
}
void VideoEncoder::SetRate(uint32_t r){
	encoder_->set_bitrate(r);
}
}
