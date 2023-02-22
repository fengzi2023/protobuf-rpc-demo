//
// Created by fengyuwei on 2022/8/10.
//

#ifndef PROTOBUF_RPC_DEMP_TCP_SERVER_H
#define PROTOBUF_RPC_DEMP_TCP_SERVER_H

#include <sys/socket.h>
#include "tcp_conn.h"


class TcpServer {
public:
    TcpServer(InetAddr addr);
    ~TcpServer();
    void start();

    void newConnect(int fd, const InetAddr&);
    void closeConnect(TcpConnPtr conn);

    Acceptor* acceptor;
    EventLoop* eventLoop;
    std::unordered_map<int, TcpConnPtr> connPtrs;
    MessageCallback messageCallback;

    void setMessageCallback(MessageCallback cb) {
        this->messageCallback=cb;
    }
};


#endif //PROTOBUF_RPC_DEMP_TCP_SERVER_H
