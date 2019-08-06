#include <stdint.h>
#include <iostream>
#include <signal.h>
#include "fake_video.h"
//using namespace webrtc;
using namespace std;
using namespace zsy;
static const int kTestHeight = 720;//1080;//288;
static const int kTestWidth = 1280;//1920;//352;
static const int kTestFramerate = 30;

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
    FakeVideoSource camera(kTestHeight,kTestWidth,kTestFramerate);
    FakeVideoEncoder sink;
    camera.RegisterSink(&sink);
    sink.StartEncoder();
    camera.StartCamera();
    while(m_running){
        
    }
    camera.StopCamera();
    sink.StopEncoder();
    return 0;
}
