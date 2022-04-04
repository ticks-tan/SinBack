/**
* FileName:   EventLoop.h
* CreateDate: 2022-03-09 09:56:06
* Author:     ticks
* Email:      2938384958@qq.com
* Des:         
*/
#ifndef SINBACK_EVENTLOOP_H
#define SINBACK_EVENTLOOP_H

#include <list>
#include <unordered_map>
#include <unordered_set>
#include "base/Logger.h"
#include "core/Selector.h"
#include "base/ThreadPool.h"

namespace SinBack
{
    namespace Core
    {
        class EventLoop final : noncopyable
        {
        public:
            EventLoop();
            ~EventLoop();

            static ULong event_loop_next_id();
            // 将事件添加到待处理链表
            static void loop_event_pending(const std::weak_ptr<Event>& ev){
                std::shared_ptr<Event> event_ptr = ev.lock();
                if (event_ptr){
                    if (!event_ptr->pending_){
                        event_ptr->pending_ = true;
                        // 事件循环待处理事件+1
                        event_ptr->loop_->pending_count_++;
                        // 将事件添加到待处理事件链表
                        event_ptr->loop_->pending_evs_[event_ptr->priority_].push_front(event_ptr);
                    }
                }
            }
            // 事件转换为活跃状态
            static void loop_event_active(const std::weak_ptr<Event>& ev){
                std::shared_ptr<Event> event_ptr = ev.lock();
                if (event_ptr){
                    if (!event_ptr->active_){
                        event_ptr->active_ = true;
                        event_ptr->loop_->actives_count_++;
                    }
                }
            }
            // 让事件处于非活跃
            static void loop_event_inactive(const std::weak_ptr<Event>& ev){
                std::shared_ptr<Event> event_ptr = ev.lock();
                if (event_ptr){
                    if (event_ptr->active_){
                        event_ptr->active_ = false;
                        event_ptr->loop_->actives_count_--;
                    }
                }
            }
            // 添加IO事件（监听某个事件）
            static Int addIoEvent(const std::weak_ptr<Core::IOEvent>& ev, const IOEventCB& cb, Int events);
            // 移除IO事件（取消监听某个事件）
            static Int removeIoEvent(const std::weak_ptr<Core::IOEvent>& ev, Int events);
            // 改变IO事件 Loop
            static bool changeIoLoop(const std::weak_ptr<Core::IOEvent>& ev, EventLoopPtr loop);
            // 从 EventLoop 中移除IO事件
            static void removeIO(const std::weak_ptr<Core::IOEvent>& ev);

            // 运行 EventLoop
            bool run();
            // 停止 EventLoop
            bool stop();
            // 暂停 EventLoop
            bool pause();
            // 恢复执行 EventLoop
            bool resume();

            // 获取 pid
            pid_t getPid() const{
                return this->pid_;
            }
            // 获取线程 pid
            pid_t getTid() const {
                return this->tid_;
            }
            // 获取 id
            UInt getId() const{
                return this->id_;
            }
            // 获取当前时间
            ULong getCurrentTime() const {
                return this->cur_time_;
            }

            // 通过套接字获取 IOEvent
            std::shared_ptr<IOEvent> getIoEvent(Base::socket_t fd);
            // 添加自定义事件
            std::shared_ptr<Core::Event> addCustom(const Core::EventCB& cb);

            template<typename Func, typename... Args>
            void runInLoop(Func&& func, Args&&... args){
                using return_type = typename std::result_of<Func(Args...)>::type;
                auto task = std::make_shared< std::packaged_task<return_type()> >(
                        std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
                );
                if (task){
                    (*task)();
                }
            }
            // 添加事件到线程池
            template<typename Func, typename... Args>
            auto queueFunc(Func&& f, Args&&... args)
            -> std::future<typename std::result_of<Func(Args...)>::type>{
                return this->thread_pool_.enqueue(std::forward<Func>(f), std::forward<Args>(args)...);
            }

            // 添加 idle事件
            std::shared_ptr<Core::IdleEvent> addIdle(const Core::IdleEventCB& cb, Int repeat);
            // 删除idle事件
            void removeIdle(const std::weak_ptr<Core::IdleEvent>& ev);

            // 添加定时器事件
            std::shared_ptr<Core::TimerEvent> addTimer(const Core::TimerEventCB& cb, UInt timeout, Int repeat);
            // 删除定时器
            void removeTimer(const std::weak_ptr<Core::TimerEvent>& ev);

            // 添加 accept 套接字
            std::shared_ptr<Core::IOEvent> acceptIo(Base::socket_t listen_fd, const Core::IOAcceptCB& cb);
            // 读取套接字
            std::shared_ptr<Core::IOEvent> readIo(Base::socket_t fd, Size_t read_len,
                                                  const Core::IOReadCB& cb, const Core::IOReadErrCB& err_cb = nullptr);
            // 写入套接字
            std::shared_ptr<Core::IOEvent> writeIo(Base::socket_t fd, const void* buf, Size_t len,
                                                   const Core::IOWriteCB& cb, const Core::IOWriteErrCB& err_cb = nullptr);
            // 关闭套接字
            void closeIo(Base::socket_t fd);

        private:
            void initLoop();
            void updateTime();

            // 处理事件
            Int processEvents();
            // 处理空闲事件
            Int processIdle();
            // 处理待处理事件
            Int processPending();
            // 处理定时器事件
            Int processTimer();
            // 处理IO事件
            Int processIo(Int timeout);

        public:
            // 默认等待时间 (ms)
            static const Int default_event_loop_wait_timeout = 1000;
            // 默认暂停等待时间 (ms)
            static const Int default_event_loop_pause_timeout = 1000;
            // 默认线程池大小
            static const Int default_thread_pool_count = 2;
            enum LoopStatus : Int {
                Stop = 1,
                Running = 2,
                Pause = 3
            };
        private:
            using EventPtr = std::shared_ptr<Event>;
            using EventPtrList = std::list<EventPtr>;

            // 进程pid
            pid_t pid_;
            // 线程pid
            pid_t tid_;
            // id
            ULong id_;
            // 当前状态
            LoopStatus status_;
            // 开始时间 (ms)
            ULong start_time_;
            // 结束时间 (ms)
            ULong end_time_;
            // 当前时间 (ms)
            ULong cur_time_;
            // 循环次数
            std::atomic<LLong> loop_count_;
            // 活跃事件数量
            Int actives_count_;

            // Selector
            std::shared_ptr<Selector> selector_;

            // 待处理事件队列
            std::array<EventPtrList , event_priority_size()> pending_evs_;
            // 待处理事件数量
            Int pending_count_;

            // IO 事件集合
            std::unordered_map<Base::socket_t, std::shared_ptr<IOEvent>> io_evs_;
            // IO事件数量
            Int io_count_;

            // 自定义事件队列
            std::queue<std::shared_ptr<Event>> custom_evs_;
            Int custom_count_;

            // 空闲事件
            std::list<std::shared_ptr<IdleEvent>> idle_evs_;
            // 空闲事件数量
            Int idle_count_;

            // 定时器事件
            std::unordered_map<ULong , std::shared_ptr<TimerEvent>> timer_;
            // 定时任务数量
            Int timer_count_;
            // 线程池
            Base::ThreadPool thread_pool_;
            // 锁
            std::mutex mutex_;
        };
    }
}


#endif //SINBACK_EVENTLOOP_H
