//
// Created by fengyuwei on 2022/8/14.
//

#include "rpc_client.h"
#include <glog/logging.h>

int TcpClient::connect() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd < 0) {
        LOG(FATAL) << "sockets::createNonblockingOrDie";
    }

    int status = ::fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
    if (status == -1) {
        perror("calling fcntl");
        // handle the error.  By the way, I've never seen fcntl fail in this way
    }

    int ret = ::connect(sockfd, (struct sockaddr *)(&addr), (socklen_t)(sizeof(struct sockaddr_in)));

    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            return 0;

        default:
            LOG(FATAL) << "Unexpected error in Connector::startInLoop " << savedErrno;
            ::close(sockfd);
            return -1;
    }
    return 0;
}

void TcpClient::connecting(int fd) {
    TcpConnPtr tcpConn = std::make_shared<TcpConn>(fd, eventLoop, nullptr, nullptr);
    tcpConn->focusEvent = EV_WRITE;
    eventLoop->addChannel(tcpConn.get());
    this->connPtr = tcpConn;
}

void unpackMessage(::google::protobuf::Message *response,
                     ::google::protobuf::Closure *done,
        TcpConnPtr conn, IOBuf &buf) {
    if (buf.size() < 1) {
        return;
    }

    char metaSize[sizeof(int)];
    std::copy(buf.begin(), buf.begin()+sizeof(int), metaSize);
    buf.erase(buf.begin(), buf.begin()+sizeof(int));

    int metaLen = *(int*)(metaSize);

    response->ParseFromArray(&(*buf.begin()), metaLen);
    buf.erase(buf.begin(), buf.begin()+metaLen);

    done->Run();
}

// done 从用户代码传递给 RPC 框架
void RpcChannel::CallMethod(const ::google::protobuf::MethodDescriptor *method, ::google::protobuf::RpcController *,
                            const ::google::protobuf::Message *request, ::google::protobuf::Message *response,
                            ::google::protobuf::Closure *done) {
    std::string serialzied_data = request->SerializeAsString();

    myrpc::RpcMeta rpc_meta;
    rpc_meta.set_service_name(method->service()->name());
    rpc_meta.set_method_name(method->name());
    rpc_meta.set_data_size(serialzied_data.size());

    std::string serialzied_str = rpc_meta.SerializeAsString();

    int serialzied_size = serialzied_str.size();
    serialzied_str.insert(0, std::string((const char*)&serialzied_size, sizeof(int)));
    serialzied_str += serialzied_data;

    IOBuf msg;
    msg.assign(serialzied_str.begin(), serialzied_str.end());
    tcpClient->send(msg, std::bind(unpackMessage, response, done, ::std::placeholders::_1, ::std::placeholders::_2));
};


