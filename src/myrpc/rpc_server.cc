//
// Created by fengyuwei on 2022/8/10.
//

#include "rpc_server.h"

#include "google/protobuf/stubs/common.h"
#include <glog/logging.h>

void RpcServer::packAndSendMessage(
        TcpConnPtr conn,
        ::google::protobuf::Message *msg) {
    int size = msg->ByteSize();
    IOBuf buf(size+sizeof(size), 0);

    std::string temp;
    temp.assign((const char*)&size,sizeof(size));
    msg->AppendToString(&temp);

    buf.assign(temp.begin(), temp.end());
    conn->send(buf);
}

void RpcServer::unpackAndProcessMessage(TcpConnPtr conn, IOBuf &buf) {
    if (buf.size() < 1) {
        return;
    }
    char metaSize[sizeof(int)];
    std::copy(buf.begin(), buf.begin()+sizeof(int), metaSize);
    buf.erase(buf.begin(), buf.begin()+sizeof(int));

    int metaLen = *(int*)(metaSize);

    myrpc::RpcMeta meta;
    meta.ParseFromArray(&(*buf.begin()), metaLen);
    buf.erase(buf.begin(), buf.begin()+metaLen);

    LOG(INFO) << "Receive service_name=" << meta.service_name();
    LOG(INFO) << "Receive method_name=" << meta.method_name();

    auto service = services[meta.service_name()].service;
    auto md = services[meta.service_name()].mds[meta.method_name()];

    LOG(INFO) << "Receive req_type=" << md->input_type()->DebugString();
    LOG(INFO) << "Receive resp_type=" << md->output_type()->DebugString();

    auto recvMsg = service->GetRequestPrototype(md).New();
    recvMsg->ParseFromArray(&(*buf.begin()), meta.data_size());
    buf.erase(buf.begin(), buf.begin()+meta.data_size());

    auto respMsg = service->GetResponsePrototype(md).New();
    auto done = ::google::protobuf::NewCallback(
            &RpcServer::packAndSendMessage,
            conn,
            respMsg
    );

    service->CallMethod(md, nullptr, recvMsg, respMsg, done);
}
