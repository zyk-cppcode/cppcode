#include "../source/Acceptor.hpp"
#include "../source/Channel.hpp"
#include "../source/logger.hpp"
#include "../source/Buffer.hpp"
#include "../source/LoopThread.hpp"
#include "../source/Connection.hpp"
#include "../source/EventLoop.hpp"
#include "../source/LoopThreadPool.hpp"
#include "../source/TcpServer.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>

// 收到客户端消息的回调
// 收到客户端消息的回调
void OnMessage(const std::shared_ptr<Connection>& conn, Buffer* buf) {
    std::string request;
    buf->read(request, buf->getReadableSize());
    //LOG(LogLevel::INFO) << "服务器收到: " << request;

    // 完全兼容 WebBench 1.5 的 HTTP/1.0 响应
    std::string response =
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 12\r\n"
        "\r\n"  // 必须的空行，分隔头部和 body
        "Hello WebBench";

    conn->Send((char*)response.c_str(), response.size());
    // 发送后立即关闭连接，让 WebBench 立刻收到完整响应
    //conn->Close();
    conn->Shutdown();
}
// void OnMessage(const std::shared_ptr<Connection>& conn, Buffer* buf) {
//     std::string msg;
//     buf->read(msg, buf->getReadableSize());
//     //std::cout<<"msg:"<<msg<<std::endl;

//     LOG(LogLevel::INFO)<< "服务器收到: " << msg;

//     // 回显给客户端（你的客户端能收到）
//     std::string reply = "server reply: " + msg;
//     conn->Send((char*)reply.data(), reply.size());
// }

// 连接建立回调
void OnConnected(const std::shared_ptr<Connection>& conn) {
    //LOG(LogLevel::INFO) << "新客户端建立连接 conn_id=" << conn->Id();
}

// 连接关闭回调
void OnClosed(const std::shared_ptr<Connection>& conn) {
    //LOG(LogLevel::INFO) << "客户端断开连接 conn_id=" << conn->Id();
}

int main()
{
    EnableConsoleLogStrategy();

    TcpServer server(8888);
    server.SetMessageCallback(OnMessage);
    server.SetConnectedCallback(OnConnected);
    server.SetClosedCallback(OnClosed);
    server.SetThreadNum(5); 
    server.SetTimeout(5); 
    server.Start();
}
