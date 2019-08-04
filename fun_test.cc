#include "yuv2jpeg.h"
#include <iostream>
#include <signal.h>
using namespace zsy;
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
    FrameToJpeg frame(15);
    frame.StartThread();
    while(m_running){

    }
    frame.StopThread();
    std::cout<<"stop main thread"<<std::endl;
}
