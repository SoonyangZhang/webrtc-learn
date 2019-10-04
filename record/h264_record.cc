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
	if(income_frames_<max_record_){
		LockScope crit(&que_lock_);
		images_.push_back(image);
		income_frames_++;
	}
	TriggerImageWriteTask();
}
void H264Record::TriggerImageWriteTask(){
	if(!task_triggered_){
		task_triggered_=true;
		WriteImageToDiskTask();
	}
}
void H264Record::WriteImageToDiskTask(){
	if(worker_){
		WriteImageToDisk();
		if(!task_done_){
			worker_->PostTask([this](){
				this->WriteImageToDiskTask();
			});
		}
	}
}
void H264Record::WriteImageToDisk(){
	
	LockScope crit(&que_lock_);
	while(!images_.empty()){
		EncodeImage image=images_.front();
		images_.pop_front();
		assert(image.size());
        uint32_t len=image.size();
		f_out_.write((const char*)image.data(),len);
		written_frames_++;
		if(written_frames_>=max_record_){
			task_done_=true;
		}
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
