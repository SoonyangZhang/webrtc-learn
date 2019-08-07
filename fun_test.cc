#include <iostream>
#include <signal.h>
#include "base/base_time.h"
#include "rtc_base/timeutils.h"
#include <sys/time.h>
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
   /* struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
	struct timeval time;
	gettimeofday(&time, NULL);   
    int64_t ms1=static_cast<int64_t>(ts.tv_sec)*1000+ts.tv_nsec/1000000;
    int64_t ms2=static_cast<int64_t>(time.tv_sec)*1000+time.tv_usec/1000;
    std::cout<<ms1<<" "<<ms2<<std::endl;  
    std::cout<<rtc::TimeMillis()<<" "<<zsy::GetMilliSeconds()
    <<" "<<base_clock64()<<" "<<base_clock32()<<std::endl;*/
    
}
