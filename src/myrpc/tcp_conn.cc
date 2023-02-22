//
// Created by fengyuwei on 2022/8/10.
//

#include <sys/uio.h>
#include <sys/socket.h>
#include <cassert>
#include <utility>
#include <arpa/inet.h>  // inet_ntop
#include <unistd.h>
#include "tcp_conn.h"

#include<glog/logging.h>
#include <sys/fcntl.h>

static int connectFd(int sockfd, sockaddr_in *addr4)
{
    socklen_t addr_len = static_cast<socklen_t>(sizeof *addr4);
    int connfd = ::accept(sockfd, (struct sockaddr *)addr4, &addr_len);
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG(FATAL) << "Socket::accept";
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG(FATAL) << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG(FATAL) << "unknown error of ::accept " << savedErrno;
                break;
        }
    }

    int status = ::fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1){
        perror("calling fcntl");
        // handle the error.  By the way, I've never seen fcntl fail in this way
    }

    char ipBuf[1024];
    inet_ntop(AF_INET, &addr4->sin_addr, ipBuf, sizeof(ipBuf));

    char buf[2048];
    snprintf(buf,sizeof buf,"Acceptor: new ipv4 connection. ip = [%s], port = [%d], socket = %d",ipBuf,ntohs(addr4->sin_port),connfd);
    LOG(INFO) << buf;

    return connfd;
}

static int createAcceptSocket(InetAddr &addr)
{
    /**
     * AF_INET（又称 PF_INET）是 IPv4 网络协议的套接字类型，
	 * AF_INET6 则是 IPv6 的；
     * AF_UNIX 则是 Unix 系统本地通信。
     * AF_INET 相比 AF_UNIX 更具通用性，因为 Windows 上有 AF_INET 而没有 AF_UNIX。
     */
    int sockfd = 0;
    sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG(FATAL) << "sockets::createNonblockingOrDie";
    }
    LOG(INFO) << "Create acceptor fd  = " << sockfd;

    int status = ::fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1){
        perror("calling fcntl");
        // handle the error.  By the way, I've never seen fcntl fail in this way
    }

    // bind
    LOG(INFO) << "Listen on port = " << ntohs(addr.sin_port);
    int ret = ::bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr));

    if (ret < 0) {
        LOG(FATAL) << "Socket bind fail. port = " << ntohs(addr.sin_port);
    }
    return sockfd;
}


void Acceptor::handleEvent(IOEvent &event) {
    if (event.revents & EV_READ) {
        InetAddr peeraddr;
        int newfd = connectFd(fd,&peeraddr);
        connectCallback(newfd, peeraddr);
    }
}

Acceptor::Acceptor(InetAddr addr, EventLoop *eventLoop, ConnectCallback cb) {
    this->addr = addr;
    this->eventLoop = eventLoop;
    this->connectCallback = std::move(cb);

    int sockfd = createAcceptSocket(this->addr);
    this->fd = sockfd;
}

int Acceptor::listen() {
    int ret = 0;
    if ((ret = ::listen(fd,SOMAXCONN)) < 0) {
        LOG(FATAL) << "listenOrDie fd = " << fd;
    }

    this->focusEvent |= EV_READ;
    eventLoop->addChannel(this);
    return ret;
}

size_t writeFd(int fd, IOBuf &writeBuf) {
    struct iovec iov{&(*writeBuf.begin()), writeBuf.size()};
    return ::writev(fd, &iov, 1);
}

size_t readFd(int fd, IOBuf &readBuf) {
    // 读取
    char buf[10000];
    struct iovec iov{&buf, 10000};
    size_t n = ::readv(fd, &iov, 1);
    readBuf.insert(readBuf.end(), buf, buf+n);

    return n;
}

void TcpConn::handleEvent(IOEvent &event) {
    if (event.revents & EV_READ) {
        int n = readFd(fd, readBuf);
        if (n==0) {
            ::close(fd);
            closeCallback(shared_from_this());
            return;
        }
        messageCallback(shared_from_this(), readBuf);
    } else if (event.revents & EV_WRITE) {
        // 写入
        if (writeBuf.size()>0) {
            int n = writeFd(fd, writeBuf);
            if (n > 0) {
                writeBuf.erase(writeBuf.begin(), writeBuf.begin() + n);
            }
        }
    }
}

void TcpConn::send(IOBuf& buf) {
    size_t n = writeFd(fd, buf);
    if (n < buf.size()) {
        focusEvent |= EV_WRITE;
        writeBuf.insert(writeBuf.end(), buf.begin()+n, buf.end());
        eventLoop->updateChannel(this);
    }
}


