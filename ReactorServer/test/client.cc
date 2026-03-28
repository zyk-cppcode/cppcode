#include "../source/Socket.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

int main() {
  Socket cli_sock;
  cli_sock.CreateClient(8888, "127.0.0.1");
  while(1){
    const char *str = "hello zyk!";
    cli_sock.Send(str, strlen(str));  
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