#include "task_queue.h"
#include "base_time.h"
#include <signal.h>
#include <iostream>
using namespace std;
using namespace zsy;
namespace zsy{
class TestTask{
public:
    TestTask(TaskQueue *worker):worker_(worker){}
    void ToTask() {
	if(count_>10)
    {return ;}
    printf("%x,%llx\n",base_clock32(),base_clock64());
    std::cout<<count_<<std::endl;
    count_++;
	worker_->PostDelayedTask([this](){
        this->ToTask();
        },500);
    }
    void Trigger(){
	worker_->PostDelayedTask([this](){
          this->ToTask();      
    },500);
    }
private:
    TaskQueue *worker_;
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
    TaskQueue worker;
    TestTask test(&worker);
    worker.Start();
    test.Trigger();
    while(m_running){
        
    }
    worker.Stop();
    return 0;
}
