#include "rtc_base/timeutils.h"
#include "videocodec.h"
#include <iostream>
namespace zsy{
VideoEncoder::VideoEncoder(int width,int height,int fps)
:width_(width),height_(height),fps_(fps),yuv_buf_(width*height*3/2){
	encoder_.reset(new H264Encoder());
}
VideoEncoder::~VideoEncoder(){
    if(running_){
    running_=false;
    /*rtc::Thread::*/Stop();
    }
    //rtc::CritScope crit(&que_lock_);
    atomic_lock(&lock_);
    while(!frames_.empty()){
    	frames_.pop_front();
    }
    atomic_unlock(&lock_);
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
			//rtc::CritScope crit(&que_lock_);
			atomic_lock(&lock_);
			if(!frames_.empty()){
				FrameTs temp=frames_.front();
				f=temp.frame;
				temp.frame=nullptr;
				enque_ts=temp.enqueTs;
				frames_.pop_front();
				que_len_--;
			}
			atomic_unlock(&lock_);
		}
		if(f){
			int width=f->width();
			int height=f->height();
			int size = width*height * 3 / 2;
			yuv_buf_.resize(size);
			
			webrtc::ConvertFromI420(*f, webrtc::kI420, 0, yuv_buf_.data());
			int out_size=0;
			int ft=0;
			uint32_t last=rtc::TimeMillis();
			encoder_->encode(yuv_buf_.data(),size
					,AV_PIX_FMT_YUV420P,image_buf_,
					&out_size,&ft,false);
			uint32_t now=rtc::TimeMillis();
			uint32_t delta=now-f->timestamp_us()/1000;
			uint32_t delta2=now-last;
			uint32_t queue=last-enque_ts;
			pic_id_++;
			std::cout<<pic_id_<<" "<<out_size<<" "<<delta<<" "<<delta2<<" "<<
			queue<<" "<<que_len_<<std::endl;
			if(sink_){
				sink_->OnEncodedImageCallBack(image_buf_,out_size,ft,delta);
			}
			delete f;
		}
	}
}
void VideoEncoder::OnFrame(const webrtc::VideoFrame& frame){
	 webrtc::VideoFrame *copy=new webrtc::VideoFrame(frame);
	 uint32_t now=rtc::TimeMillis();
	 //rtc::CritScope crit(&que_lock_);
	 atomic_lock(&lock_);
	 frames_.push_back(FrameTs(copy,now));
	 que_len_++;
	 atomic_unlock(&lock_);
}
void VideoEncoder::SetRate(uint32_t r){
	encoder_->set_bitrate(r);
}
}
