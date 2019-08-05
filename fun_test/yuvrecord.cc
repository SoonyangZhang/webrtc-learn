#include <iostream>
#include <string>
#include <stdio.h>
#include <memory>
#include "yuvrecord.h"
//#include "api/scoped_refptr.h"
#include "api/video/video_frame_buffer.h"
namespace zsy{
FrameToFile::FrameToFile(uint32_t max):max_record_(max){
	std::string name("frame_info.txt");
	info_.open(name.c_str(), std::fstream::out);
}

FrameToFile::~FrameToFile(){
	info_.close();
}
void FrameToFile::StartThread(){
	if(!running_){
		running_=true;
		rtc::Thread::Start();
	}
}
void FrameToFile::StopThread(){
	if(running_){
		running_=false;
		rtc::Thread::Stop();
	}
}
void FrameToFile::Run(){
	while(running_){
		WriteFrameToFile();
	}
}
void FrameToFile::OnFrame(const webrtc::VideoFrame& frame){
	int f_w=frame.width();
	int f_h=frame.height();
        int ms=frame.timestamp_us()/1000;
	WritePicInfo(f_w,f_h,ms);
	if(pic_id_<=max_record_){
		 webrtc::VideoFrame copy=frame;
		 rtc::CritScope crit(&que_lock_);
		 frames_.push_back(copy);
	}
}
/*

 */
void FrameToFile::WriteFrameToFile(){
	rtc::CritScope crit(&que_lock_);
	if(!frames_.empty()){
		webrtc::VideoFrame frame=frames_.front();
		frames_.pop_front();
		rtc::scoped_refptr<webrtc::I420BufferInterface> i420_buffer=
				 frame.video_frame_buffer()->ToI420();
		int f_w=frame.width();
		int f_h=frame.height();
		int width=i420_buffer->width();
		int height=i420_buffer->height();
		std::string name=std::to_string(width)+"X"+
				std::to_string(height)+"_"+
				std::to_string(pic_id_)+".yuv";
		pic_id_++;
		std::ofstream out;
		out.open(name.c_str(),std::ofstream::binary);
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


		std::unique_ptr<uint8_t > videoImage;
		int size = width*height * 3 / 2;
		videoImage.reset(new uint8_t[size]);

		webrtc::ConvertFromI420(frame, webrtc::kI420, 0, videoImage.get());
		out.write((const char*)(videoImage.get()), size);
		out.close();
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
}
