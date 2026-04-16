/*长连接测试1：创建一个客户端持续给服务器发送数据，直到超过超时时间看看是否正常*/
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cassert>
#include "../../server/Socket.hpp"
#include "../../server/logger.hpp"

int main()
{
     EnableConsoleLogStrategy();
    Socket cli_sock;
    cli_sock.CreateClient(8111, "127.0.0.1");
    std::string req = "GET /hello.txt HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
    while(1) {
        ssize_t ret = cli_sock.Send(req.c_str(), req.size());
        if (ret <= 0) {
            LOG(LogLevel::ERROR) << "发送失败，连接已断开！";
            break; 
        }
        // ------------------ 接收 ------------------
        char buf[1024] = {0};
        ret = cli_sock.Recv(buf, 1023);
        if (ret < 0) {
            LOG(LogLevel::ERROR) << "接收异常！";
            break;
        }
        if (ret == 0) {
            LOG(LogLevel::ERROR) << "服务器关闭连接！";
            break;
        }
        std::cout<<buf<<std::endl<<std::endl;
        sleep(3);
    }
    cli_sock.Close();
    return 0;
}