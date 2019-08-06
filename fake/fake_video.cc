#include "fake_video.h"
#include <unistd.h>
namespace zsy{
static int buff_id=0;
FakeBuffer::FakeBuffer(int size):buf_(new uint8_t [size]){
    id_=buff_id;
    buff_id++;
}
FakeFrame CreateFakeFrame(int len){

    FakeFrame frame(len);
    printf("%s\n",__FUNCTION__);
    return frame;
}
FakeVideoSource::FakeVideoSource(int width,int height,int fps)
:width_(width),height_(height),fps_(fps){
    interval_=1000/fps_;
}
FakeVideoSource::~FakeVideoSource(){
    if(running_){
        StopCamera();
    }
}
void FakeVideoSource::StartCamera(){
    if(!running_){
        running_=true;
        rtc::Thread::Start();
    }
}
void FakeVideoSource::StopCamera(){
    if(running_){
        running_=false;
        rtc::Thread::Stop();
    }
}
void FakeVideoSource::Run(){
    while(running_){
        int64_t now=rtc::TimeMillis();
        if(now>last_ts_){
            int len=width_*height_*3/2;
            FakeFrame f(len);
            for(auto it=sinks_.begin();it!=sinks_.end();it++){
                (*it)->OnIncomeData(f);
            }
           last_ts_=now+interval_;
        }
    }
}
void FakeVideoSource::RegisterSink(FakeSinkInterface<FakeFrame>*sink){
    bool exist=false;
    for(auto it=sinks_.begin();it!=sinks_.end();it++){
        if((*it)==sink){
            exist=true;
            break;
        }
    }
    if(!exist){
        sinks_.push_back(sink);
    }
}
void FakeVideoSource::RemoveSink(FakeSinkInterface<FakeFrame>*sink){
    sinks_.remove_if([sink](const FakeSinkInterface<FakeFrame>* v){
                     return v==sink;
                     });
}
FakeVideoEncoder::~FakeVideoEncoder(){
    if(running_){
        StopEncoder();
    }
    rtc::CritScope crit(&que_lock_);
    while(!frames_.empty()){
        FakeFrame *f=nullptr;
        FakeFrameTs *temp=frames_.front();
		f=temp->frame;
    	frames_.pop_front();
	delete temp;
    	delete f;
    }
}
void FakeVideoEncoder::StartEncoder(){
    if(!running_){
        running_=true;
        rtc::Thread::Start();
    }
}
void FakeVideoEncoder::StopEncoder(){
    if(running_){
         running_=false;
        rtc::Thread::Stop();
    }
}
void FakeVideoEncoder::Run(){
    while(running_){
        FakeFrame *f=nullptr;
        uint32_t enque_ts=0;
        if(que_len_>0){
            rtc::CritScope crit(&que_lock_);
            if(frames_.size()){
                FakeFrameTs *temp=frames_.front();
				f=temp->frame;
				temp->frame=nullptr;
				enque_ts=temp->enqueTs;
				frames_.pop_front();
				delete temp;
				que_len_--;
            }
        }
        if(f){
            uint32_t last=rtc::TimeMillis();
            //emulate encoder delay
            usleep(40000);//40ms
            uint32_t now=rtc::TimeMillis();
            uint32_t delta=now-last;
	    uint32_t queue_d=last-enque_ts;
	    printf("%d,%d\n",delta,queue_d);
	    delete f;
        }
    }
}
void FakeVideoEncoder::OnIncomeData(const FakeFrame &frame){
    FakeFrame *copy=new FakeFrame(frame);
    uint32_t now=rtc::TimeMillis();
    rtc::CritScope crit(&que_lock_);
    if(!frames_.empty()){
    FakeFrameTs *last_fs=frames_.back();
    uint32_t delta=now-last_fs->enqueTs;
   // printf("d %d\n",delta);
	}

    FakeFrameTs *fs=new FakeFrameTs(copy,now);
    frames_.push_back(fs);
    que_len_++;
}
}
