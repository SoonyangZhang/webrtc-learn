#pragma once
#include <stdint.h>
#include <pthread.h>
namespace zsy{
class MyThread{
public:
    bool Start();
    void Stop();
    void Sleep(int ms);
    virtual void Run()=0;
private:
    pthread_t pid_;
};
}
