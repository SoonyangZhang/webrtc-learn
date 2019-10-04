#pragma once
#include "video_header.h"
#include <list>
#include <iostream>
namespace zsy{
class VideoCaptureCallback:
		public rtc::VideoSinkInterface<webrtc::VideoFrame> {

public:
    bool AddSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink){
        bool success=false;
        bool exist=false;
        for(auto it=sinks_.begin();it!=sinks_.end();it++){
            if((*it)==sink){
                exist=true;
                break;
            }
        }
        if(!exist){
            sinks_.push_back(sink);
            success=true;
        }
        return success;
    }
    bool RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink){
        bool success=false;
        for(auto it=sinks_.begin();it!=sinks_.end();){
            if((*it)==sink){
                it=sinks_.erase(it);
                success=true;
                break;
            }else{
                it++;
            }
        }
        return success;
    }
	void OnFrame(const webrtc::VideoFrame& videoFrame) override {
	    int height = videoFrame.height();
	    int width = videoFrame.width();
	    uint32_t size=videoFrame.size();
	    uint64_t now_ms=rtc::Time();
	    uint64_t record_ms=0;
	    if(last_time_==0){
	    	record_ms=0;
	    }else
	    {
	    	record_ms=now_ms-last_time_;
	    }
	    //std::cout<<frame_num_<<" "<<record_ms<<" "<<size<<" "<<height<<" "<<width<<std::endl;
	    last_time_=now_ms;
	    frame_num_++;
        for(auto it=sinks_.begin();it!=sinks_.end();it++){
            (*it)->OnFrame(videoFrame);
        }
	}
private:
	int frame_num_{0};
	uint64_t last_time_{0};
    std::list<rtc::VideoSinkInterface<webrtc::VideoFrame>*> sinks_;
};    
}
