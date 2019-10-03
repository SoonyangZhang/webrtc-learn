#pragma once
#include <fstream>
#include <iostream>
#include <deque>
#include <map>
#include "video_bridge.h"
#include "task_queue.h"
#include "lock.h"
#include "base/buffer.h"
#include "base/atomic_ops.h"
#include "base/scoped_refptr.h"
/*
 * jpeg libtool command not found
https://blog.csdn.net/a593796769/article/details/9211697
https://blog.csdn.net/Meteor_s/article/details/82588613
*/
namespace zsy{
class FrameBuffer{
public:
    FrameBuffer(int w,int h,const uint8_t*data,size_t size);
    ~FrameBuffer(){}
    int get_ref(){
        return buf_->GetRefCount();
    }
    int Height(){return h_;}
    int Width(){return w_;}
    int size(){return buf_->size();}
    uint8_t *data() {return buf_->data();}
private:
    int w_;
    int h_;
    scoped_refptr<RefCountedObject<Buffer>> buf_;
};
class FrameToFile :public rtc::VideoSinkInterface<webrtc::VideoFrame>{
public:
	FrameToFile(TaskQueue *worker,uint32_t max);
	~FrameToFile();
	void OnFrame(const webrtc::VideoFrame& frame) override;
	void OnFrameBeforeEncode(const webrtc::VideoFrame& frame);
	void OnFrameBeforeEncode(int w,int h,const uint8_t *data,size_t size);
    void OnFrameAfterDecode(uint32_t frame_id,int w,int h,const uint8_t *data,size_t size);
	void MayWriteFrameToDisk();
	void CalculateQoE();
private:
	void WriteFrameToFile();
	void WriteFrameToFile(const uint8_t *data,size_t size,int w,int h);
	void EraseFrameBefore(uint32_t id);
	void WritePicInfo(int w,int h,int ms);
	void WriteQoEInfo(uint32_t frame_id,double ssim,double psnr);
    TaskQueue *worker_{nullptr};
	uint32_t pic_id_{0};
	uint32_t frame_to_disk_{0};
	uint32_t max_record_{1};
    std::fstream info_;
    std::fstream qoe_info_;
	bool pic_info_{true};
    AtomicLock que_lock_;
    std::deque<webrtc::VideoFrame> frames_;
    AtomicLock frame_lock_;
    std::map<uint32_t,FrameBuffer> encode_frames_;
    std::map<uint32_t,FrameBuffer> decode_frames_;
    uint32_t encode_frame_id_{1};
    //uint32_t deocde_frame_id_{1};
    uint32_t QoEEncodeFrames_{1};
    uint32_t QoEDecodeFrames_{1};
    uint32_t QoEMaxFrames_{100};
};
}
