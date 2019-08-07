#pragma once
#include <fstream>
#include <iostream>
#include <deque>
#include "video_bridge.h"
#include "task_queue.h"
#include "lock.h"
/*
 * jpeg libtool command not found
https://blog.csdn.net/a593796769/article/details/9211697
https://blog.csdn.net/Meteor_s/article/details/82588613
*/
namespace zsy{
class FrameToFile :public rtc::VideoSinkInterface<webrtc::VideoFrame>{
public:
	FrameToFile(TaskQueue *worker,uint32_t max);
	~FrameToFile();
	void OnFrame(const webrtc::VideoFrame& frame) override;
    void MayWriteFrameToDisk();
private:
	void WriteFrameToFile();
	void WritePicInfo(int w,int h,int ms);
    TaskQueue *worker_{nullptr};
	uint32_t pic_id_{1};
	uint32_t max_record_{1};
    std::fstream info_;
	bool pic_info_{true};
    AtomicLock que_lock_;
    std::deque<webrtc::VideoFrame> frames_;

};
}
