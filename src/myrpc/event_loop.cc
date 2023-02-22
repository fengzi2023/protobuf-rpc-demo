//
// Created by fengyuwei on 2022/8/10.
//

#include "event_loop.h"
#include "selector.h"
#include <glog/logging.h>

void EventLoop::runLoop() {
    _stop = false;
    std::vector<IOEvent> events;
    // loop
    for(;;) {
        if (_stop) {
            break;
        }
        events.clear();
        selector->select(events);
        for (auto & event : events) {
            Channel *channel = channels.find(event.fd)->second;
            channel->handleEvent(event);
        }
        runTasks();
    }
}

void EventLoop::runTasks() {
    std::vector<IOTask> pendingTasks;
    {
        std::lock_guard<std::mutex> guard(mutex);
        pendingTasks.swap(this->tasks);
    }
    for (auto & task : pendingTasks) {
        task();
    }
}

Channel::~Channel() {
    LOG(INFO) << "~Channel: fd = " << fd;
    // ::close(fd);
}
