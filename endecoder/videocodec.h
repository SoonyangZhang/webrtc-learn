#pragma once
#include <memory>
#include "rtc_base/thread.h"
#include "rtc_base/criticalsection.h"
#include "base/base_time.h"
#include "base/atomic_ops.h"
#include "base/buffer.h"
#include "base/scoped_refptr.h"
#include "base/my_thread.h"
#include "base/lock.h"
#include "video_header.h"
#include "echo_h264_encoder.h"
#include "H264Decoder.h"
namespace zsy{
struct FrameTs{
  FrameTs(webrtc::VideoFrame *f,uint32_t ts):frame(f),enqueTs(ts){}	
  webrtc::VideoFrame *frame;
  uint32_t enqueTs;
};
class EncodeImage{
public:
    EncodeImage(const uint8_t*data,size_t len,uint32_t ft,
                uint32_t capture_ts,uint32_t encode_ts);
    ~EncodeImage(){}
    EncodeImage(const EncodeImage&)=default;
    EncodeImage& operator=(const EncodeImage&)=default;
    //! caution, use copy,not move.
    EncodeImage(EncodeImage&&r){
        *this=std::move(r);
    }
    EncodeImage& operator=(EncodeImage&&r){
        ft_=r.ft_;
        capture_ts_=r.capture_ts_;
        encode_ts_=r.encode_ts_;
        buffer_=std::move(r.buffer_);
        return *this;
    }
    const uint8_t *data() const;
    uint8_t *data();
    uint32_t size() const{
        return buffer_->size();
    }
    uint32_t FrameType()const{
        return ft_;
    }
    uint32_t CaptureTs() const{
        return capture_ts_;
    }
    uint32_t EncodeTs() const{
        return encode_ts_;
    }
private:
    uint32_t ft_;
    uint32_t capture_ts_;
    uint32_t encode_ts_;
    scoped_refptr<RefCountedObject<Buffer>> buffer_;
};
class EncodedVideoCallback{
public:
	virtual ~EncodedVideoCallback(){}
	virtual void OnEncodedImageCallBack(EncodeImage &image)=0;
};
class YUVBuffer{
public:
    YUVBuffer(){}
	YUVBuffer(int size):size_(size){
        if(size_){
            data_.reset(new uint8_t[size_]);
        }
    }
	void resize(int len){
		if(len>size_){
			data_.reset(new uint8_t[len]);
			size_=len;
		}
	}
    uint32_t size() const {return size_;}
	uint8_t* data(){return (uint8_t*)(data_.get());}
private:
	int size_{0};
	std::unique_ptr<uint8_t[]>data_;

};
class VideoEncoder :public MyThread,public rtc::VideoSinkInterface<webrtc::VideoFrame>{
public:
	VideoEncoder(int width,int height,int fps);
	~VideoEncoder();
	void StartEncoder();
	void StopEncoder();
	void Run() override;
	void OnFrame(const webrtc::VideoFrame& frame) override;
	bool RegisterSink(EncodedVideoCallback *cb);
    void RegisterYUVRecord(rtc::VideoSinkInterface<webrtc::VideoFrame> *yuv);
	void SetRate(uint32_t);
private:
	bool running_{false};
	int width_;
	int height_;
	int fps_;
	std::unique_ptr<H264Encoder> encoder_;
	rtc::CriticalSection que_lock_;
	std::list<FrameTs> frames_;
	std::list<EncodedVideoCallback*> sinks_;
    rtc::VideoSinkInterface<webrtc::VideoFrame> *yuv_{nullptr};
	YUVBuffer yuv_buf_;
	uint8_t image_buf_[1024000];
    int pic_id_{0};
	int total_pid_{0};
	AtomicLock lock_;
	int que_len_{0};
    int max_que_{4};
    int max_que_delay_{100};
};
class VideoDecoder:public MyThread,public EncodedVideoCallback{
public:
    VideoDecoder(uint32_t h,uint32_t w);
    ~VideoDecoder();
    void StartDecoder();
    void StopDecoder();
    void Run() override;
    void ResetHxW(uint32_t height,uint32_t width){
        uint32_t len=height*width*3/2;
        yuv_buf_.resize(len);
    }
    void OnEncodedImageCallBack(EncodeImage &image) override;
private:
    uint32_t height_{0};
    uint32_t width_{0};
    bool running_{false};
    AtomicLock que_lock_;
    std::list<EncodeImage> images_;
    uint32_t que_len_{0};
    YUVBuffer yuv_buf_;
    H264Decoder decoder_;
	X264_DECODER_H handler_;
    uint32_t image_count_{0};
    uint32_t max_record_{5};
    uint32_t record_id_{0};
    uint32_t decode_id_{0};
};
}
