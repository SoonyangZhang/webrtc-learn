#include "h264_record.h"
#include <iostream>
namespace zsy{
H264Record::H264Record(TaskQueue *worker,std::string &s){
	worker_=worker;
	std::string name=s+".h264";
	f_out_.open(name.c_str(),std::ofstream::binary);
}
H264Record::~H264Record(){
	WriteImageToDisk();
	f_out_.close();
}
void H264Record::OnEncodedImageCallBack(uint8_t *data,uint32_t size,int frametype,
		uint32_t capture_ts,uint32_t encode_ts)
{
	EncodeImage image(data,size,frametype,capture_ts,encode_ts);
	LockScope crit(&que_lock_);
	images_.push_back(std::move(image));
	if(worker_){
		worker_->PostTask([this](){
			this->MayWriteImageToDisk();
		});
	}
}
void H264Record::MayWriteImageToDisk(){
	WriteImageToDisk();
}
void H264Record::WriteImageToDisk(){
	
	LockScope crit(&que_lock_);
	while(!images_.empty()){
		EncodeImage image=std::move(images_.front());
		images_.pop_front();
		assert(image.size());
		std::cout<<"write image "<<image.size()<<std::endl;
		f_out_.write((const char*)image.data(),image.size());
	}
}
}
