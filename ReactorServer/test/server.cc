// #include "../source/Socket.hpp"
// #include <iostream>

#include "../source/Poller.hpp"
#include "../source/Channel.hpp"
#include "../source/Socket.hpp"
#include "../source/Buffer.hpp"
#include "../logger.hpp"


#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

int main() {
    EnableConsoleLogStrategy();
    // 1. 创建 Poller(epoll)
    Poller poller;

    // 2. 创建监听 socket
    Socket listen_sock;
    if (!listen_sock.CreateServer(8888)) {  // 绑定 0.0.0.0:8888
        LOG(LogLevel::ERROR) << "创建服务器失败";
        return -1;
    }
    listen_sock.setNonBlock();      // 非阻塞

    // 3. 创建监听 Channel
    Channel channel(&poller, listen_sock.fd());
    channel.SetReadCallBack([&]() {
        // 新连接到来
        int newfd = listen_sock.Accept();
        if (newfd < 0) return;

        // 为新连接设置非阻塞
        Socket client_sock(newfd);
        client_sock.setNonBlock();

        // 为新连接创建 Channel
        Channel* client_channel = new Channel(&poller, newfd);
        client_channel->SetReadCallBack([=]() {
            // 读事件
            char buf[1024];
            ssize_t n = ::recv(newfd, buf, sizeof(buf), 0);
            if (n <= 0) {
                LOG(LogLevel::INFO) << "客户端断开 fd=" << newfd;
                client_channel->RemoveEvent();
                delete client_channel;
                ::close(newfd);
                return;
            }
            LOG(LogLevel::INFO) << "收到数据: " << std::string(buf, n);
            ::send(newfd, "server recv ok\n", 14, 0);
        });

        // 启动读事件
        client_channel->StartReadEvent();
    });

    // 开始监听读事件
    channel.StartReadEvent();

    // 4. 事件循环
    LOG(LogLevel::INFO) << "服务器启动成功 0.0.0.0:8888";
    while (true) {
        std::vector<Channel*> active;
        poller.poll(active);
        for (Channel* ch : active) {
            ch->HandleEvent();
        }
    }

    return 0;
}
// int main() {
//   Socket lst_sock;
//   lst_sock.CreateServer(8500);
//   while (1) {
//     int newfd = lst_sock.Accept();
//       std::cout << "newfd: " << newfd << std::endl;
//     if (newfd < 0) {
//       std::cout << "Accept failed" << std::endl;
//       continue;
//     }
//       Socket cli_sock(newfd);
//       char buf[1024] = {0};
//       int ret = cli_sock.Recv(buf, 1023);
//       std::cout << "recv ret: " << ret << std::endl;
//       if (ret < 0) {
//         cli_sock.Close();
//         continue;}
//         std::cout << "recv: " << buf << std::endl;
//         cli_sock.Send(buf, ret);
//         cli_sock.Close();
//         //lst_sock.Close();
      
//     }
  
//   return 0;
//}