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
#include "rtc_send_stream.h"
#include "rtc_recv_stream.h"
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
    VideoEncoder encoder(kTestWidth,kTestHeight,kTestFramerate);
	video_capture_.AddSink(&encoder);
    FrameToFile yuv_record(&worker,5);
	std::string name=std::to_string(kTestWidth)+"x"+std::to_string(kTestHeight);
	H264Record h264sink(&worker,name);
	encoder.RegisterSink(&h264sink);
    encoder.RegisterYUVRecord(&yuv_record);
    VideoDecoder decoder(kTestHeight,kTestWidth);
    decoder.StartDecoder();
    encoder.RegisterSink(&decoder);

    RTCSendStream sender;
    std::string packet_log("packet.txt");
    RTCRecvStream receiver(packet_log);
    sender.RegisterReceiver(&receiver);
    encoder.RegisterSink(&sender);
    sender.StartSender();
	video_capture_.StartCapture(capability);
	encoder.StartEncoder();
	while(m_running)
	{
		
	}
	printf("stop capture\n");
	video_capture_.StopCapture();
	encoder.StopEncoder();
    decoder.StopDecoder();
    worker.Stop();
    sender.StopSender();
	return 0;
}
