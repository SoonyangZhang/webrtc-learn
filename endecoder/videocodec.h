#pragma once
#include <memory>
#include "rtc_base/thread.h"
#include "rtc_base/criticalsection.h"
#include "video_header.h"
#include "echo_h264_encoder.h"
#include "H264Decoder.h"
namespace zsy{
class EncodedVideoCallback{
public:
	virtual ~EncodedVideoCallback(){}
	virtual void OnEncodedImageCallBack(uint8_t *image,uint32_t size,int frametype,uint32_t delta)=0;
};
class VideoEncoder :public rtc::Thread,public rtc::VideoSinkInterface<webrtc::VideoFrame>{
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
	std::deque<webrtc::VideoFrame> frames_;
	EncodedVideoCallback *sink_{nullptr};
	uint8_t image_buf_[1024000];
        int pic_id_{0};
};
}
