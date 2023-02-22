//
// Created by fengyuwei on 2022/8/10.
//

#ifndef PROTOBUF_RPC_DEMP_TCP_CONN_H
#define PROTOBUF_RPC_DEMP_TCP_CONN_H


#include <utility>
#include <netinet/in.h>
#include "event_loop.h"
#include <functional>

class TcpConn;
typedef std::shared_ptr<TcpConn> TcpConnPtr;
typedef std::vector<char> IOBuf;
typedef struct sockaddr_in InetAddr;

typedef std::function<void (int,const InetAddr&)> ConnectCallback;
typedef std::function<void (TcpConnPtr, IOBuf&)> MessageCallback ;
typedef std::function<void (TcpConnPtr)> CloseCallback ;

class Acceptor : Channel {
public:
    Acceptor(InetAddr addr, EventLoop *eventLoop, ConnectCallback cb);
    virtual void handleEvent(IOEvent &event) override;
    int listen();

    ConnectCallback connectCallback;
    InetAddr addr;
};

class TcpConn : public Channel, public std::enable_shared_from_this<TcpConn>
        {
public:
    TcpConn(int fd, EventLoop *eventLoop, MessageCallback cb, CloseCallback closeCallback) {
        this->fd = fd;
        this->eventLoop = eventLoop;
        this->messageCallback = cb;
        this->closeCallback = closeCallback;
        this->focusEvent = EV_READ | EV_WRITE;
    }

    void handleEvent(IOEvent &event) override;

public:
    void send(IOBuf&);

    MessageCallback messageCallback;
    CloseCallback closeCallback;
    IOBuf readBuf;
    IOBuf writeBuf;
};


#endif //PROTOBUF_RPC_DEMP_TCP_CONN_H
