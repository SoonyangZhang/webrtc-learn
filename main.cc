/*
 * video_capture.cc
 *
 *  Created on: 2018.6.22
 *      Author: zsy
 *      email 865678017@qq.com
 *      This file is modified from video_capture_unittest.cc
 * https://cs.chromium.org/chromium/src/third_party/webrtc/
 */
#include <stdint.h>
#include <iostream>
#include <signal.h>
#include <string>
#include "video_capture.h"
#include "yuv_record.h"
#include "h264_record.h"
#include "videocodec.h"
#include "task_queue.h"
using namespace webrtc;
using namespace std;
using namespace zsy;
static const int kTestHeight = 720;//1080;//288;
static const int kTestWidth = 1280;//1920;//352;
static const int kTestFramerate = 22;//30;

bool m_running=true;
void signal_exit_handler(int sig)
{
	m_running=false;
} 
int main()
{
	signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);
    TaskQueue worker;
    worker.Start();
	zsy::VideoCapture video_capture_;
	video_capture_.SetUp() ;
	video_capture_.OpenVideoCaptureDevice();
	webrtc::VideoCaptureCapability capability;
	video_capture_.GetCapability(capability);
    capability.width = kTestWidth;
    capability.height = kTestHeight;
   	capability.maxFPS = kTestFramerate;
   	FrameToFile sink(&worker,2);
    	VideoEncoder encoder(kTestWidth,kTestHeight,kTestFramerate);
   	video_capture_.AddSink(&sink);
	video_capture_.AddSink(&encoder);

	std::string name=std::to_string(kTestWidth)+"x"+std::to_string(kTestHeight);
	H264Record h264sink(&worker,name);
	encoder.RegisterSink(&h264sink);
	video_capture_.StartCapture(capability);
	encoder.StartEncoder();
	while(m_running)
	{
		
	}
	printf("stop capture\n");
	video_capture_.StopCapture();
	encoder.StopEncoder();
    worker.Stop();
	return 0;
}
