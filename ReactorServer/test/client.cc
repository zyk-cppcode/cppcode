#include "../Socket.hpp"
#include <iostream>

int main() {
  Socket cli_sock;
  cli_sock.CreateClient(8500, "127.0.0.1");
  for (int i = 0; i < 5; i++) {
    const char *str = "hello zyk!";
    cli_sock.Send(str, strlen(str));  // 正确，直接传字符串指针和长度
    char buf[1024] = {0};
    cli_sock.Recv(buf, 1023);
    // spdlog::info("Received: {}", buf);
    std::cout << "Received: " << buf << std::endl;
    sleep(1);
  }
  // while (1)
  //   sleep(1);
  return 0;
}