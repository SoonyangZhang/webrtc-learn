#include <iostream>
#include <string>
#include <stdio.h>
#include <memory>
#include <utility>
#include "yuv_record.h"
#include "tools/psnr.h"
#include "api/video/video_frame_buffer.h"
namespace zsy{
FrameBuffer::FrameBuffer(int w,int h,const uint8_t*data,size_t size){
	w_=w;
	h_=h;
	buf_=new RefCountedObject<Buffer>(size);
	if(data){
		buf_->SetData(data,size);
	}
}
FrameToFile::FrameToFile(TaskQueue *worker,uint32_t max)
:worker_(worker),max_frames_to_disk_(max){
	std::string name("qoe_info.txt");
	qoe_info_.open(name.c_str(), std::fstream::out);
}

FrameToFile::~FrameToFile(){
	//info_.close();
	qoe_info_.close();
}
void FrameToFile::OnFrame(const webrtc::VideoFrame& frame){
	if(incoming_frame_<max_frames_to_disk_){
		webrtc::VideoFrame copy=frame;
		LockScope crit(&que_lock_);
		frames_.push_back(copy);
		incoming_frame_++;
	}
	TriggerFrameToDiskTask();
	OnFrameBeforeEncode(frame);
}
void FrameToFile::OnFrameBeforeEncode(const webrtc::VideoFrame& frame){
	if(QoEEncodeFrames_<QoEMaxFrames_){
		LockScope crit(&frame_lock_);
		rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer=
				 frame.video_frame_buffer()->ToI420();
		int width=i420_buffer->width();
		int height=i420_buffer->height();
		int size = width*height * 3 / 2;
		FrameBuffer buffer(width,height,nullptr,size);
		webrtc::ConvertFromI420(frame, webrtc::kI420, 0, buffer.data());
		encode_frames_.insert(std::make_pair(QoEEncodeFrames_,buffer));
		QoEEncodeFrames_++;
	}
}
void FrameToFile::OnFrameBeforeEncode(int w,int h,const uint8_t *data,size_t size){

}
void FrameToFile::OnFrameAfterDecode(uint32_t frame_id,int w,int h,const uint8_t *data,size_t size){
	if(QoEDecodeFrames_<QoEMaxFrames_){
		LockScope crit(&frame_lock_);
		FrameBuffer buffer(w,h,data,size);
		decode_frames_.insert(std::make_pair(QoEDecodeFrames_,buffer));
		QoEDecodeFrames_++;
	}
	TriggerQoETask();
}
void FrameToFile::EraseFrameBefore(uint32_t id){
	LockScope crit(&frame_lock_);
	while(!decode_frames_.empty()){
		auto it=decode_frames_.begin();
		if(it->first<=id){
			decode_frames_.erase(it);
		}else{
			break;
		}
	}
	while(!encode_frames_.empty()){
		auto it=encode_frames_.begin();
		if(it->first<=id){
			encode_frames_.erase(it);
		}else{
			break;
		}
	}
}
void FrameToFile::CalculateQoE(){
	uint32_t frame_id=0;
	{
		LockScope crit(&frame_lock_);
		if(!decode_frames_.empty()){
			auto it=decode_frames_.begin();
			frame_id=it->first;
			FrameBuffer decode_frame=it->second;
			auto it2=encode_frames_.find(frame_id);
			if(it2!=encode_frames_.end()){
				FrameBuffer encode_frame=it2->second;
				int h=decode_frame.Height();
				int w=decode_frame.Width();
				double ssim=::x264_pixel_ssim_wxh(encode_frame.data(),w,decode_frame.data(),w,w,h);
				int inc=1,Y=h*w*3/2;
				double psnr=::calculate_psnr(encode_frame.data(),decode_frame.data(),inc,Y);
				WriteQoEInfo(frame_id,ssim,psnr);
				qoe_calculate_++;
				if(qoe_calculate_>=QoEMaxFrames_){
					qoe_task_done_=true;
				}
			}
		}
	}
	if(frame_id>0){
		EraseFrameBefore(frame_id);
	}
}
void FrameToFile::TriggerFrameToDiskTask(){
	if(!frame_task_triggered_){
		FrameToDiskTask();
		frame_task_triggered_=true;
	}
}
void FrameToFile::TriggerQoETask(){
	if(!qoe_task_triggered_){
		QoETask();
		qoe_task_triggered_=true;
	}
}
void FrameToFile::FrameToDiskTask(){
		if(worker_){
			WriteFrameToFile();
			if(!frames_written_done_){
		        worker_->PostTask([this](){
		            this->FrameToDiskTask();
		        });
			}
	    }
}
void FrameToFile::QoETask(){
	if(worker_){
	CalculateQoE();
	if(!qoe_task_done_){
		worker_->PostTask([this](){
		     this->QoETask();
		});
			}
	}
}
void FrameToFile::WriteFrameToFile(){
        LockScope crit(&que_lock_);
        if(!frames_.empty()){
		webrtc::VideoFrame frame=frames_.front();
		frames_.pop_front();
		rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer=
				 frame.video_frame_buffer()->ToI420();
		int f_w=frame.width();
		int f_h=frame.height();
		int width=i420_buffer->width();
		int height=i420_buffer->height();


		std::unique_ptr<uint8_t> videoImage;
		int size = width*height * 3 / 2;
		videoImage.reset(new uint8_t[size]);
		webrtc::ConvertFromI420(frame, webrtc::kI420, 0, videoImage.get());
		WriteFrameToFile(videoImage.get(), size,width,height);
		//rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer=
		//		frame.video_frame_buffer();
		/*int y=i420_buffer->StrideY();
		int u=i420_buffer->StrideU();
		int v=i420_buffer->StrideV();
		int y_plan=y*height;
		int u_plan=u*((height+1)/2);
		int v_plan=u_plan;
		out.write(reinterpret_cast<const char*>(i420_buffer->DataY()), y_plan);
		out.write(reinterpret_cast<const char*>(i420_buffer->DataU()), u_plan);
		out.write(reinterpret_cast<const char*>(i420_buffer->DataV()), v_plan);*/
	}
}
void FrameToFile::WriteFrameToFile(const uint8_t *data,size_t size,int w,int h){
	std::string name=std::to_string(w)+"X"+
			std::to_string(h)+"_"+
			std::to_string(frames_written_)+".yuv";
	frames_written_++;
	std::ofstream out;
	out.open(name.c_str(),std::ofstream::binary);
	out.write((const char*)(data),size);
	out.close();
	if(frames_written_>=max_frames_to_disk_){
		frames_written_done_=true;
	}
}
void FrameToFile::WritePicInfo(int w,int h,int ms){
	if(pic_info_){
		char line [512];
		memset(line,0,512);
		sprintf (line, "%d %d %d",
				w,h,ms);
		info_<<line<<std::endl;
	}
}
void FrameToFile::WriteQoEInfo(uint32_t frame_id,double ssim,double psnr){
	std::string delimit(std::string("\t"));
	std::string info=std::to_string(frame_id)+delimit+
			std::to_string(ssim)+delimit+
			std::to_string(psnr);
	qoe_info_<<info<<std::endl;
}
}
