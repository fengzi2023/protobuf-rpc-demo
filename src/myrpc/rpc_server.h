//
// Created by fengyuwei on 2022/8/10.
//

#ifndef PROTOBUF_RPC_DEMP_RPC_SERVER_H
#define PROTOBUF_RPC_DEMP_RPC_SERVER_H

#include "rpc_meta.pb.h"
#include "tcp_server.h"
#include "google/protobuf/service.h"

class RpcServer {
public:
    RpcServer(TcpServer *tcpServer) {
        this->tcpServer = tcpServer;
    }

    void addService(::google::protobuf::Service* service) {
        ServiceInfo service_info;
        service_info.service = service;
        service_info.sd = service->GetDescriptor();
        for (int i = 0; i < service_info.sd->method_count(); ++i) {
            service_info.mds[service_info.sd->method(i)->name()] = service_info.sd->method(i);
        }

        services[service_info.sd->name()] = service_info;
    }

    void start() {
        tcpServer->setMessageCallback(std::bind(&RpcServer::unpackAndProcessMessage, this, std::placeholders::_1, std::placeholders::_2));
        tcpServer->start();
    }

private:
    static void packAndSendMessage(TcpConnPtr conn,
                            ::google::protobuf::Message *msg);

    void unpackAndProcessMessage(TcpConnPtr conn, IOBuf &buf);

    struct ServiceInfo{
        ::google::protobuf::Service* service;
        const ::google::protobuf::ServiceDescriptor* sd;
        std::map<std::string, const ::google::protobuf::MethodDescriptor*> mds;
    };//ServiceInfo

    //service_name -> {Service*, ServiceDescriptor*, MethodDescriptor* []}
    std::map<std::string, ServiceInfo> services;

    TcpServer *tcpServer;
};


#endif //PROTOBUF_RPC_DEMP_RPC_SERVER_H
