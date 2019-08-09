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
