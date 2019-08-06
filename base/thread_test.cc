#include "my_thread.h"
#include <signal.h>
#include <iostream>
using namespace std;
using namespace zsy;
namespace zsy{
class TestTask:public MyThread{
public:
    void StartTask(){
	running_=true;
        MyThread::Start();
    }
    void StopTask(){
	running_=false;
        MyThread::Stop();
    }
    void Run() override{
	while(running_){
        std::cout<<count_<<std::endl;
        count_++;
        Sleep(100);
	}
    }
private:
    bool running_{false};
    int count_{0};
};    
}
bool m_running=true;
void signal_exit_handler(int sig)
{
	m_running=false;
} 

int main(){
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler);
    TestTask task;
    task.StartTask();
    while(m_running){
        
    }
    task.StopTask();
    return 0;
}
