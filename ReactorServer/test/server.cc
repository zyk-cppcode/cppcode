
#include "../source/Acceptor.hpp"
#include "../source/Channel.hpp"
#include "../source/logger.hpp"
#include "../source/Buffer.hpp"
#include "../source/Connection.hpp"
#include "../source/EventLoop.hpp"
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <functional>


// void HandleClose(Channel *channel) {
//   LOG(LogLevel::INFO)<< "close fd: " << channel->Fd();
//   //std::cout << "close fd: " << channel->Fd() << std::endl;
//   channel->Remove();   // 先从epoll移除
//   close(channel->Fd());     // 关闭fd
//   //delete channel;           // 释放channel
//   channel->GetLoop()->QueueInLoop([channel]() {
//         delete channel;
//     });
// }

// void HandleRead(Channel *channel) {

//   int fd = channel->Fd();
//   if(fd<0)
//   return;
//   char buf[1024] = {0};
//   ssize_t ret = recv(fd, buf, 1023, 0);

//   if (ret <= 0) {
//     HandleClose(channel);
//     return;
//   }
//   LOG(LogLevel::INFO)<< "recv from " << fd << " : " << buf;
//   //std::cout << "recv from " << fd << " : " << buf << std::endl;
//   // 收到数据后触发发送
//   channel->EnableWrite();
// }

// void HandleWrite(Channel *channel) {
//   int fd = channel->Fd();
//   const char *data = "你好吗";
//   ssize_t ret = send(fd, data, strlen(data), MSG_NOSIGNAL);

//   if (ret < 0) {
//     HandleClose(channel);
//     return;
//   }

//   // 发送完关闭写事件，避免一直触发
//   channel->DisableWrite();
// }

// void HandleError(Channel *channel) {
//   HandleClose(channel);
// }



// // ==============================
// // 新连接回调
// // ==============================
// void Acceptor(EventLoop*loop, Channel *lst_channel) {
//   int listen_fd = lst_channel->Fd();
//   int newfd = accept(listen_fd, nullptr, nullptr);
//   if (newfd < 0) return;
//   uint64_t id=rand()%1000;
//   std::cout << "新连接：" << newfd <<" id="<<id<< std::endl;

//   Connection *conn = new Connection(newfd,listen_fd,loop);
//   conn->SetMessageCallback(std::bind());
//   conn->SetConnectedCallback(std::bind());
//   conn->SetClosedCallback(std::bind());
//   conn->SetEventCallback(std::bind());
// }

// int main() {
//   EnableConsoleLogStrategy();
//   std::cout << "========= 服务器启动 ==========" << std::endl;
//   EventLoop loop;
//   Socket lst_sock;
//   lst_sock.CreateServer(8888);
//   std::cout << "===================" << std::endl;

//   Channel channel(&loop, lst_sock.Fd());
//   channel.SetReadCallBack(std::bind(Acceptor, &loop, &channel));
//   channel.EnableRead();
  
//   while (1) {
//   std::cout << "=========inloop==========" << std::endl;

//     loop.Loop();
//   }

//   return 0;
// }
    EventLoop loop;
int conn_id = 1;
// 收到客户端消息的回调
void OnMessage(const std::shared_ptr<Connection>& conn, Buffer* buf) {
    std::string msg;
    buf->read(msg, buf->getReadableSize());
    //std::cout<<"msg:"<<msg<<std::endl;

    LOG(LogLevel::INFO)<< "服务器收到: " << msg;

    // 回显给客户端（你的客户端能收到）
    std::string reply = "server reply: " + msg;
    conn->Send((char*)reply.data(), reply.size());
}

// 连接建立回调
void OnConnected(const std::shared_ptr<Connection>& conn) {
    LOG(LogLevel::INFO) << "新客户端建立连接 conn_id=" << conn->Id();
}

// 连接关闭回调
void OnClosed(const std::shared_ptr<Connection>& conn) {
    LOG(LogLevel::INFO) << "客户端断开连接 conn_id=" << conn->Id();
}

// 新连接到达
void OnNewConnection(int client_fd) {
 
    auto conn = std::make_shared<Connection>(conn_id++, client_fd, &loop);

    conn->SetMessageCallback(OnMessage);
    conn->SetConnectedCallback(OnConnected);
    conn->SetClosedCallback(OnClosed);
    conn->EnableInactiveRelease(5); // 5秒无活动自动释放连接
    conn->Established(); // 启动读事件
}

int main() {
    EnableConsoleLogStrategy();
    
    Acceptor acceptor(&loop, 8888); // 创建Acceptor监听端口
        // // 监听 8888 端口
        // Socket listen_sock;
        // listen_sock.CreateServer(8888);
    acceptor.SetAcceptCallBack(std::bind(OnNewConnection, std::placeholders::_1));

    LOG(LogLevel::INFO)<< "服务器启动，监听 8888 端口...";
    while(1){
    loop.Loop();

    }
   
    return 0;
}