#include "task_queue.h"
#include "base_time.h"
namespace zsy
{
void ThreadMain(void* context)
{
    TaskQueue *task=static_cast<TaskQueue*>(context);
    task->Process();
}
TaskEvent::TaskEvent(uint32_t d,std::unique_ptr<QueuedTask>t){
	delay=d;
	task=std::move(t);
}
TaskEvent::~TaskEvent(){}
int TaskEvent::operator>(TaskEvent &b){
	return delay>b.delay;
}

TaskQueue::TaskQueue():thread_(ThreadMain,this)
{
	min_heap_ctor(&s_heap_);
        Start();
}
TaskQueue::~TaskQueue(){
	Clear();
	min_heap_dtor(&s_heap_);
}
void TaskQueue::Clear(){
	while(!min_heap_empty(&s_heap_)){
	TaskEvent *e=min_heap_top(&s_heap_);
	min_heap_pop(&s_heap_);
	delete e;
	}
}
bool TaskQueue::IsCurrent() const{
	return (&thread_)==ThreadManager::Instance()->CurrentThread();
}
void TaskQueue::Start()
{
    m_running=true;
    thread_.Start();
}
void TaskQueue::Stop()
{
    m_running=false;
    thread_.Stop();
}
void TaskQueue::Process()
{
   //well that seems better,make the lock part as small as possible
	while(m_running)
	{
        uint64_t time_ms=GetMilliSeconds();
        std::list<std::unique_ptr<QueuedTask>> pending_tasks;
        {
           
            LockScope lock(&pending_lock_);
            while(!min_heap_empty(&s_heap_)){
            TaskEvent *event=min_heap_top(&s_heap_);
            if(event->delay<=time_ms){
                pending_tasks.push_back(std::move(event->task));
                delete event;
                min_heap_pop(&s_heap_);
            }
            else{
                break;
                }
            }
	    }
	    while(!pending_tasks.empty())
	    {
	        std::unique_ptr<QueuedTask> task=std::move(pending_tasks.front());//move ownership
	        pending_tasks.pop_front();
	        task->Run();
	        //task.release();// this will cause memory leakgy
	    }            
    }
	    
}
void TaskQueue::PostTask(std::unique_ptr<QueuedTask>task)
{
    uint64_t delta=0;
	PostDelayedTask(std::move(task),delta);
}
void TaskQueue::PostDelayedTask(std::unique_ptr<QueuedTask> task, uint32_t time_ms)
{

    uint64_t now=GetMilliSeconds();
    uint64_t future=now+time_ms;
    TaskEvent *event=new TaskEvent(future,std::move(task));
    LockScope lock(&pending_lock_);
    min_heap_push(&s_heap_,event);
}
}
