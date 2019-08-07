#pragma once
#include <string>
#include <list>
#include <memory>
#include <fstream>
#include <string.h>
#include <assert.h>
#include "lock.h"
#include "videocodec.h"
#include "task_queue.h"
namespace zsy{
class EncodeImage{
public:
    EncodeImage(uint8_t*data,uint32_t len,uint32_t ft,
                uint32_t capture_ts,uint32_t encode_ts){
    size_=len;
    ft_=ft;
    capture_ts_=capture_ts;
    encode_ts_=encode_ts;
    if(len){
        uint8_t *buf=new uint8_t [len];
        memcpy((void*)buf,(void*)data,len);
        image_.reset(buf);
    }
}
   ~EncodeImage(){
   }
    EncodeImage(const EncodeImage&)=delete;
    EncodeImage & operator=(const EncodeImage&)=delete;
    EncodeImage(EncodeImage&&r){
        *this=std::move(r);
    }
    EncodeImage &operator=(EncodeImage&&r){
        image_=std::move(r.image_);
        size_=r.size_;
        r.size_=0;
	ft_=r.ft_;
        capture_ts_=r.capture_ts_;
        encode_ts_=r.encode_ts_;
        return (*this);

    }
    uint8_t *data() const{
        return (uint8_t*)image_.get();
    }
    uint32_t size() const{
        return size_;
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
    std::unique_ptr<uint8_t []> image_;
    uint32_t size_;
    uint32_t ft_;
    uint32_t capture_ts_;
    uint32_t encode_ts_;
};
class H264Record:public EncodedVideoCallback{
public:
	H264Record(TaskQueue *worker,std::string &s);
	~H264Record();
	void OnEncodedImageCallBack(uint8_t *data,uint32_t size,int frametype,
			uint32_t capture_ts,uint32_t encode_ts) override;
	void MayWriteImageToDisk();
private:
	void WriteImageToDisk();
	TaskQueue *worker_;
	std::ofstream f_out_;
    std::fstream info_;
    int32_t first_encode_ts_{-1};
    uint32_t first_capture_ts_{0};
	AtomicLock que_lock_;
	std::list<EncodeImage> images_;
};
}
