#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include "yuv2jpeg.h"
//#include "api/scoped_refptr.h"
#include "api/video/video_frame_buffer.h"
namespace zsy{
FrameToJpeg::~FrameToJpeg(){

}
void FrameToJpeg::StartThread(){
	if(!running_){
		running_=true;
		rtc::Thread::Start();
	}
}
void FrameToJpeg::StopThread(){
	if(running_){
		running_=false;
		rtc::Thread::Stop();
	}
}
void FrameToJpeg::Run(){
	while(running_){
		WriteFrameToFile();
	}
}
void FrameToJpeg::OnFrame(const webrtc::VideoFrame& frame){
	if(pic_id_<=max_record_){
		 webrtc::VideoFrame copy=frame;
		 rtc::CritScope crit(&que_lock_);
		 frames_.push_back(copy);
	}
}
/*
int size = width*height * 3 / 2;//32 / 8;
videoImage_.reset(new uint8_t[size]);
webrtc::VideoFrame &f = (data->f_);
webrtc::ConvertFromI420(f, webrtc::kI420, 0, videoImage_.get());
fout_.write((const char*)(videoImage_.get()), size);
 */
void FrameToJpeg::WriteFrameToFile(){
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
		int y=i420_buffer->StrideY();
		int u=i420_buffer->StrideU();
		int v=i420_buffer->StrideV();
		int y_plan=y*height;
		int u_plan=u*((height+1)/2);
		int v_plan=u_plan;
		out.write(reinterpret_cast<const char*>(i420_buffer->DataY()), y_plan);
		out.write(reinterpret_cast<const char*>(i420_buffer->DataU()), u_plan);
		out.write(reinterpret_cast<const char*>(i420_buffer->DataV()), v_plan);
		out.close();
		WritePicInfo(f_w,f_h,width,height,y,u,v);
	}
}
void FrameToJpeg::WritePicInfo(int f_w,int f_h,int w,int h,int y,int u,int v){
	if(pic_info_){
		std::string name("frame_info.txt");
		std::fstream info;
		info.open(name.c_str(), std::fstream::out);
		char line [512];
		memset(line,0,512);
		sprintf (line, "%d %16d %16d %16d %16d %16d %16d",
				f_w,f_h,w,h,y,u,v);
		info<<line<<std::endl;
		info.close();
		pic_info_=false;
	}
}
}
