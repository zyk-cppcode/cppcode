#include "../source/Socket.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>



int main() {
    // 1. 创建客户端 socket 并连接服务器
    Socket cli_sock;
    if (!cli_sock.CreateClient(8888, "127.0.0.1")) {
        std::cerr << "连接服务器失败！" << std::endl;
        return -1;
    }


    // 2. 循环发送数据并接收响应
    while (true) {
        const char *send_str = "hello zyk!";
        ssize_t send_len = cli_sock.Send(send_str, strlen(send_str));
        if (send_len <= 0) {
            std::cerr << "发送失败，连接断开！" << std::endl;
            break;
        }

        char buf[1024] = {0};
  std::cout << "===================" << std::endl;

        ssize_t recv_len = cli_sock.Recv(buf, sizeof(buf) -1);
  std::cout << recv_len << std::endl;
        if (recv_len <= 0) {
            std::cout << "接收失败，连接断开！" << std::endl;
           break;
        }


        std::cout << "Received: " << buf << std::endl;
        sleep(1);
    }
  //std::cout << "=1==================" << std::endl;

    return 0;
}
// int main() {
//   Socket cli_sock;
//   cli_sock.CreateClient(8888, "127.0.0.1");
//   while(1){
//     const char *str = "hello zyk!";
//     cli_sock.Send(str, strlen(str));  
//     char buf[1024] = {0};
//     cli_sock.Recv(buf, 1023);
    
//     std::cout << "Received: " << buf << std::endl;
//     sleep(1);
//   }
//   // while (1)
//   //   sleep(1);
//   return 0;
// }