#include <iostream>
#include "echo.pb.h"
#include "myrpc/rpc_client.h"

void printEcho(std::shared_ptr<echo::EchoResponse> resp, EventLoop *eventLoop) {
    std::cerr << resp->msg() << std::endl;

    // 在 IO 回调中，关闭 EventLoop，结束代码循环。
    eventLoop->stop();
}

int main() {
    InetAddr addr4;
    addr4.sin_family = AF_INET;
    addr4.sin_port = htons(8081);
    addr4.sin_addr.s_addr = inet_addr("127.0.0.1");

    EventLoop* eventLoop = new EventLoop();
    TcpClient* tcpClient = new TcpClient(addr4, eventLoop);

    tcpClient->connect();

    RpcChannel* rpcChannel = new RpcChannel(tcpClient);

    // 为了让 eventLoop 执行，所以将代码异步执行。
    // 这里也是创建一个 std::function 对象，通过 addIOTask 复制给 EventLoop tasks 成员变量中。
    eventLoop->addIOTask([rpcChannel, eventLoop] {
        echo::EchoRequest request;
        auto response = std::make_shared<echo::EchoResponse>();
        request.set_msg("hello, myrpc.");

        echo::EchoService_Stub stub(rpcChannel);

        // 在堆上创建一个 Closure，Run完成之后，会删除这个对象。
        // response 是 Closure 的成员，shared_ptr 会把对象引用传递给 Closure 保存。直到 run 执行之后，Closure 删除会释放。
        auto cb = ::google::protobuf::NewCallback(printEcho, response, eventLoop);

        stub.Echo(nullptr, &request, response.get(), cb);
    });

    eventLoop->runLoop();

    return 0;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 */
