//
// Created by fengyuwei on 2022/8/10.
//

#ifndef PROTOBUF_RPC_DEMP_SELECTOR_H
#define PROTOBUF_RPC_DEMP_SELECTOR_H

#include <vector>


class Channel;
struct IOEvent {
    int revents;
    int fd;
    void *extraPtr;
};

const int EV_NONE = 0x00,EV_READ = 0x01,EV_WRITE = 0x02,EV_UNKNOWN = 0x04,IO_EV_ERROR = 0x08, EV_CLOSE = 0x10;

/**
 * Demultiplexer
 *  事件分离器，epoll/poll
 */
class Selector {
public:
    virtual int select(std::vector<IOEvent>& evs) = 0;
    virtual bool add(int fd, int ev) = 0;
    virtual bool remove(int fd) = 0;
    virtual bool update(int fd, int ev) = 0;
};

extern Selector * makeSelector();

#endif //PROTOBUF_RPC_DEMP_SELECTOR_H
