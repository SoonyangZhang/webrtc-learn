#include "my_thread.h"
#include <unistd.h>
namespace zsy{
void *thread_fun(void *param){
    MyThread *thread=static_cast<MyThread*>(param);
    thread->Run();
}
bool MyThread::Start(){
    int ret=0;
    ret=pthread_create(&pid_,nullptr,thread_fun,(void*)this);
    if(ret){
        return false;
    }
    return true;

}
void MyThread::Stop(){
    pthread_join(pid_,nullptr);
}
void MyThread::Sleep(int ms){
    uint64_t us=ms*1000;
    usleep(us);
}
}
