//
// Created by fengyuwei on 2022/8/14.
//

#ifndef PROTOBUF_RPC_DEMP_RPC_CLIENT_H
#define PROTOBUF_RPC_DEMP_RPC_CLIENT_H

#include <iostream>
#include "rpc_meta.pb.h"
#include "google/protobuf/service.h"
#include "google/protobuf/stubs/common.h"
#include "boost/asio.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/make_shared.hpp"
#include "event_loop.h"
#include "tcp_conn.h"

class TcpClient: boost::noncopyable {
    EventLoop *eventLoop;
    InetAddr addr;
    MessageCallback messageCallback;
    TcpConnPtr connPtr;
public:
    TcpClient(InetAddr addr, EventLoop *eventLoop) {
        this->addr = addr;
        this->eventLoop = eventLoop;
    }
    void send(IOBuf &buf, MessageCallback cb) {
       connPtr->send(buf);
       connPtr->focusEvent |= EV_READ;
       connPtr->messageCallback = cb;
       eventLoop->updateChannel(connPtr.get());
    }
    int connect();
    void connecting(int fd);
};

class RpcChannel: public ::google::protobuf::RpcChannel {
    TcpClient *tcpClient;
public:
    RpcChannel(TcpClient *tcpClient) {
       this->tcpClient = tcpClient;
    }

    virtual void CallMethod(const ::google::protobuf::MethodDescriptor* method,
                            ::google::protobuf::RpcController* /* controller */,
                            const ::google::protobuf::Message* request,
                            ::google::protobuf::Message* response,
                            ::google::protobuf::Closure* done) override;


};


#endif //PROTOBUF_RPC_DEMP_RPC_CLIENT_H
