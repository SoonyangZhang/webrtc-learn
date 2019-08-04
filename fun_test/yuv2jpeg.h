#pragma once
#include "rtc_base/thread.h"
#include "rtc_base/criticalsection.h"
#include "video_bridge.h"
#include <iostream>
#include <deque>
/*
 * jpeg libtool command not found
https://blog.csdn.net/a593796769/article/details/9211697
https://blog.csdn.net/Meteor_s/article/details/82588613
*/
namespace zsy{
class FrameToJpeg :public rtc::Thread,public rtc::VideoSinkInterface<webrtc::VideoFrame>{
public:
	FrameToJpeg(uint32_t max):max_record_(max){}
	~FrameToJpeg();
	void StartThread();
	void StopThread();
	void Run() override;
	void OnFrame(const webrtc::VideoFrame& frame) override;
private:
	void WriteFrameToFile();
	void WritePicInfo(int f_w,int f_h,int w,int h,int y,int u,int v);
	rtc::MessageList msgs_;
	bool running_{false};
	uint32_t pic_id_{1};
	uint32_t max_record_{1};
	rtc::CriticalSection que_lock_;
	std::deque<webrtc::VideoFrame> frames_;
	bool pic_info_{true};
};
}
