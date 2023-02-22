//
// Created by fengyuwei on 2022/8/10.
//

#include "tcp_server.h"
#include <glog/logging.h>
#include <netinet/in.h>
#include <functional>

//void onMessage(TcpConnPtr conn, IOBuf &buf) {
//   LOG(INFO) << "Receive from : fd=" << conn->fd << " msg = " << std::string(buf.begin(), buf.end());
//   IOBuf wr(buf);
//   buf.clear();
//   conn->send(wr);
//}

TcpServer::TcpServer(InetAddr addr4) {


    this->eventLoop = new EventLoop();
    this->acceptor = new Acceptor(
            addr4,
            eventLoop,
            std::bind(&TcpServer::newConnect, this, std::placeholders::_1, std::placeholders::_2)
    );
}

void TcpServer::start() {
    this->acceptor->listen();
    this->eventLoop->runLoop();
}


TcpServer::~TcpServer() {
   // this->acceptor->close();
}


void TcpServer::newConnect(int fd, const InetAddr &) {
    auto closeCallback = std::bind(&TcpServer::closeConnect, this, std::placeholders::_1);
    TcpConnPtr conn = std::make_shared<TcpConn>(fd, eventLoop, messageCallback, closeCallback);
    connPtrs[fd] = conn;
    eventLoop->addChannel(conn.get());
}

void TcpServer::closeConnect(TcpConnPtr conn) {
    eventLoop->removeChannel(conn.get());
    connPtrs.erase(conn->fd);
}


//int main(int argc,char *argv[]) {
//    TcpServer * server = new TcpServer();
//    server->start();
//}
