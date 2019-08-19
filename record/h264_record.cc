#include "h264_record.h"
#include <iostream>
namespace zsy{
H264Record::H264Record(TaskQueue *worker,std::string &s){
	worker_=worker;
	std::string name=s+".h264";
	f_out_.open(name.c_str(),std::ofstream::binary);
    name=s+"_encode.txt";
    info_.open(name.c_str(),std::fstream::out);
}
H264Record::~H264Record(){
	WriteImageToDisk();
	f_out_.close();
    info_.close();
}
void H264Record::OnEncodedImageCallBack(EncodeImage &image)
{
	LockScope crit(&que_lock_);
	images_.push_back(image);
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
		EncodeImage image=images_.front();
		images_.pop_front();
		assert(image.size());
        uint32_t len=image.size();
		f_out_.write((const char*)image.data(),len);
        if(first_encode_ts_==-1){
            first_encode_ts_=image.EncodeTs();
            first_capture_ts_=image.CaptureTs();
        }
        uint32_t delta1=image.CaptureTs()-first_capture_ts_;
        uint32_t delta2=image.EncodeTs()-first_encode_ts_;
        uint32_t ft=image.FrameType();
		char line [512];
		memset(line,0,512);
		sprintf (line, "%d %d %d %d",
				delta1,delta2,len,ft);
		info_<<line<<std::endl;        
	}
}
}
