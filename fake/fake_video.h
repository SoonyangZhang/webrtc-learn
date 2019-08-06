#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <list>
#include "rtc_base/thread.h"
#include "rtc_base/timeutils.h"
namespace zsy{
template <class T>
class FakeSinkInterface{
public:
    virtual void OnIncomeData(const T& data)=0;
    virtual ~FakeSinkInterface(){}
};
class FakeBuffer{
public:
    FakeBuffer(int size);
    ~FakeBuffer(){
        delete [] buf_;
        //printf("%d buf delete\n",id_);
    }
private:
    uint8_t* buf_{nullptr};
    int id_;
};
class FakeFrame{
public:
    FakeFrame(int size):yuv_(new FakeBuffer(size)){}
    FakeFrame(const FakeFrame&)=default;
    FakeFrame& operator=(const FakeFrame&)=default;
    FakeFrame(FakeFrame&&)=default;
    FakeFrame& operator=(FakeFrame&&)=default;
    ~FakeFrame(){}
private:
    std::shared_ptr<FakeBuffer> yuv_;
};
struct FakeFrameTs{
    FakeFrameTs(FakeFrame *f,uint32_t now):
    frame(f),enqueTs(now){}
    ~FakeFrameTs(){
        frame=nullptr;
    }
    FakeFrame *frame;
    uint32_t enqueTs{0};
};
FakeFrame CreateFakeFrame(int len);
class FakeVideoSource:public rtc::Thread{
public:
    FakeVideoSource(int width,int height,int fps);
    ~FakeVideoSource();
    void StartCamera();
    void StopCamera();
    void Run() override;
    void RegisterSink(FakeSinkInterface<FakeFrame>*sink);
    void RemoveSink(FakeSinkInterface<FakeFrame>*sink);
private:
    int width_;
    int height_;
    int fps_;
    int interval_;
    std::list<FakeSinkInterface<FakeFrame>*> sinks_;
    int64_t last_ts_{-1};
    bool running_{false};
};
class FakeVideoEncoder:public FakeSinkInterface<FakeFrame>,public rtc::Thread{
public:
    FakeVideoEncoder(){}
    ~FakeVideoEncoder();
    void StartEncoder();
    void StopEncoder();
    void Run() override;
    void OnIncomeData(const FakeFrame &frame) override;
private:
	rtc::CriticalSection que_lock_;
	std::list<FakeFrameTs*> frames_;
	bool running_{false};
    int que_len_{0};
};
}
