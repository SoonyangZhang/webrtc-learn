#pragma once
#include <memory>
#include "rtc_base/thread.h"
#include "rtc_base/criticalsection.h"
#include "video_header.h"
#include "echo_h264_encoder.h"
#include "H264Decoder.h"
#include "my_thread.h"
#include "lock.h"
namespace zsy{
struct FrameTs{
  FrameTs(webrtc::VideoFrame *f,uint32_t ts):frame(f),enqueTs(ts){}	
  webrtc::VideoFrame *frame;
  uint32_t enqueTs;
};
class EncodedVideoCallback{
public:
	virtual ~EncodedVideoCallback(){}
	virtual void OnEncodedImageCallBack(uint8_t *image,uint32_t size,int frametype,uint32_t capture_ts,uint32_t encode_ts)=0;
};
class YUVBuffer{
public:
	YUVBuffer(int size):size_(size),data_(new uint8_t[size_]){}
	void resize(int len){
		if(len>size_){
			data_.reset(new uint8_t[len]);
			size_=len;
		}
	}
	uint8_t* data(){return (uint8_t*)(data_.get());}
private:
	int size_{0};
	std::unique_ptr<uint8_t[]>data_;
	
};
class VideoEncoder :public MyThread/*rtc::Thread*/,public rtc::VideoSinkInterface<webrtc::VideoFrame>{
public:
	VideoEncoder(int width,int height,int fps);
	~VideoEncoder();
	void StartEncoder();
	void StopEncoder();
	void Run() override;
	void OnFrame(const webrtc::VideoFrame& frame) override;
	void RegisterSink(EncodedVideoCallback *cb){
		sink_=cb;
	}
	void SetRate(uint32_t);
private:
	bool running_{false};
	int width_;
	int height_;
	int fps_;
	std::unique_ptr<H264Encoder> encoder_;
	rtc::CriticalSection que_lock_;
	std::list<FrameTs> frames_;
	EncodedVideoCallback *sink_{nullptr};
	YUVBuffer yuv_buf_;
	uint8_t image_buf_[1024000];
        int pic_id_{0};
	int total_pid_{0};
	int lock_{0};
	int que_len_{0};
        int max_que_{4};
};
}
