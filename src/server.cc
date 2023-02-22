#include "myrpc/rpc_server.h"
#include "echo.pb.h"

class MyEchoService : public echo::EchoService {
public:
  virtual void Echo(::google::protobuf::RpcController* /* controller */,
                       const ::echo::EchoRequest* request,
                       ::echo::EchoResponse* response,
                       ::google::protobuf::Closure* done) {
      std::cout << request->msg() << std::endl;
      response->set_msg(
              std::string("I have received '") + request->msg() + std::string("'"));
      done->Run();
  }
};//MyEchoService

int main() {
    InetAddr addr4;
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(8081);
    addr4.sin_addr.s_addr = INADDR_ANY;

    TcpServer* tcpServer = new TcpServer(addr4);
    RpcServer* rpcServer = new RpcServer(tcpServer);
    MyEchoService echo_service;
    rpcServer->addService(&echo_service);
    rpcServer->start();

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
