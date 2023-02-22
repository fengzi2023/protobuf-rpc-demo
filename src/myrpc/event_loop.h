//
// Created by fengyuwei on 2022/8/10.
//

#ifndef PROTOBUF_RPC_DEMP_EVENT_LOOP_H
#define PROTOBUF_RPC_DEMP_EVENT_LOOP_H

#include <vector>
#include<deque>
#include<unordered_map>
#include "selector.h"
#include <mutex>
#include <boost/core/noncopyable.hpp>

class EventLoop;

/**
 * Handle
 *  持有系统网络连接描述符和事件执行逻辑
 */
class Channel: boost::noncopyable {
public:
    virtual void handleEvent(IOEvent &event) = 0;
    int fd; // handle
    int focusEvent; // 关注的event
    EventLoop* eventLoop;

    ~Channel();
};

// Demultiplexer
class Selector;

/**
 * Reactor
 *  线程执行主体，IO 多路复用主循环
 */
class EventLoop {
public:
    typedef std::function<void()> IOTask;
    EventLoop(): selector(makeSelector()), _stop(true) {
    };
    void runLoop();

    void addChannel(Channel *ch) {
        selector->add(ch->fd, ch->focusEvent);
        channels[ch->fd] = ch;
    }

    void updateChannel(Channel *ch) {
       selector->update(ch->fd, ch->focusEvent);
    }

    void removeChannel(Channel *ch) {
        selector->remove(ch->fd);
        channels.erase(ch->fd);
    }

    void addIOTask(IOTask task) {
        std::lock_guard<std::mutex> guard(mutex);
        tasks.push_back(task);
    }

    void stop() {
        _stop = true;
    }

private:
    std::unordered_map<int, Channel*> channels;
    Selector *selector;

    std::mutex mutex;
    std::vector<IOTask> tasks;

    bool _stop;

    void runTasks();
};


#endif //PROTOBUF_RPC_DEMP_EVENT_LOOP_H
