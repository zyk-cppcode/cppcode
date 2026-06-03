#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../server/logger.hpp"


int main() {
    EnableConsoleLogStrategy();

    // 1. 创建 socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // 2. 服务器地址：本机 8080 端口（和你 server 一致）
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // 3. 连接
    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // 4. 发送 HTTP GET 请求
    std::string request =
        "GET /index.html HTTP/1.1\r\n"
        "Host: 127.0.0.1\r\n"
        "Connection: close\r\n"
        "\r\n";

    write(sockfd, request.data(), request.size());
    //LOG(LogLevel::DEBUG)<<"Sent HTTP request:\n"<<request;
    // 5. 读取响应
    char buf[4096];
    ssize_t n;
    while ((n = read(sockfd, buf, sizeof(buf)-1)) > 0) {
        buf[n] = '\0';
        std::cout << buf;
    }

    std::cout << std::endl;
    close(sockfd);
    return 0;
}