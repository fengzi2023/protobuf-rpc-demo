//
// Created by fengyuwei on 2022/8/10.
//

#include "selector.h"
#include <poll.h>
#include <unordered_map>
#include <glog/logging.h>

class PollSelector : public Selector {
public: PollSelector() {

}
private:
    std::vector<pollfd> fds;
    std::unordered_map<int, size_t> fdToIdx;

    int select(std::vector<IOEvent> &events) override;
    bool add(int fd, int ev) override;
    bool update(int fd, int ev) override;
    bool remove(int fd) override;
};

static int transEv(int revents) {
    int e = EV_NONE;
    if (revents & POLLIN) {
        e |= EV_READ;
    }
    if (revents & POLLOUT) {
        e |= EV_WRITE;
    }
    return e;
}

int PollSelector::select(std::vector<IOEvent> &events) {
    int active;
    active = ::poll(&(*fds.begin()),fds.size(), 50);
    if (active >= 0) {
        if (0 == active) {
            return -1;
        }
        //LOG(INFO) << "active events = " << active << ", total = " << fds.size();
        for (auto pfd = fds.begin(); pfd != fds.end() && active > 0; ++pfd) {
            if (pfd->revents > 0) {
                --active;
                IOEvent event = {transEv(pfd->revents),pfd->fd};
                events.push_back(event);
                //LOG(INFO) << "event: fd = " << event.fd << " revents = " << (event.revents & EV_READ ? "R":"W") << (event.revents & EV_WRITE ? "R":"W");
            }
        }
    } else {
        // Io error
        LOG(FATAL) << "Io Error!";
        return -2;
    }
    return 0;
}

bool PollSelector::add(int fd, int ev) {
    short pollev = ((EV_READ & ev) ? POLLIN : 0) | ((EV_WRITE & ev) ? POLLOUT : 0);
    pollfd pfd{fd,pollev,0};
    fds.push_back(pfd);
    LOG(INFO) << "Add fd = " << pfd.fd << ",focus_event = "<< (ev & EV_READ ? "R":"") << (ev & EV_WRITE ? "W":"");
    LOG(INFO) << "fds.size = " << fds.size();
    fdToIdx[fd] = fds.size()-1;
    return true;
}

bool PollSelector::update(int fd,int ev) {
    auto idx = fdToIdx.find(fd);
    if (idx == fdToIdx.end()) {
        LOG(ERROR) << "Fail to find fd:" << fd;
        return false;
    }
    LOG(INFO) << "Update fd = " << fd << ",focus_event = "<< (ev & EV_READ ? "R":"") << (ev & EV_WRITE ? "W":"");
    short pollev = ((EV_READ & ev) ? POLLIN : 0) | ((EV_WRITE & ev) ? POLLOUT : 0);
    fds[idx->second].events = pollev;
    return true;
}

bool PollSelector::remove(int fd) {
    auto idx = fdToIdx.find(fd);
    if (idx == fdToIdx.end()) {
        LOG(ERROR) << "Fail to find fd:" << fd;
        return false;
    }
    LOG(INFO) << "Remove fd = " << idx->first << " ,idx = " << idx->second;
    if (idx->second < fdToIdx.size()-1) {
        // 待删除fd与最后一个元素交换
        LOG(INFO) << "_pollfds.back().fd = " << fds.back().fd << ", fd = " << fd;
        std::iter_swap(fds.end()-1,fds.begin()+idx->second);
        fdToIdx[fds.back().fd] = idx->second;
    }
    fds.pop_back();
    fdToIdx.erase(fd);
    return 0;
}


Selector* makeSelector() {
    return new PollSelector();
}
