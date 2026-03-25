#include "../Socket.hpp"

int main() {
  Socket lst_sock;
  lst_sock.CreateServer(8500);
  while (1) {
    int newfd = lst_sock.Accept();
      std::cout << "newfd: " << newfd << std::endl;
    if (newfd < 0) {
      std::cout << "Accept failed" << std::endl;
      continue;
    }
      Socket cli_sock(newfd);
      char buf[1024] = {0};
      int ret = cli_sock.Recv(buf, 1023);
      std::cout << "recv ret: " << ret << std::endl;
      if (ret < 0) {
        cli_sock.Close();
        continue;}
        std::cout << "recv: " << buf << std::endl;
        cli_sock.Send(buf, ret);
        cli_sock.Close();
        //lst_sock.Close();
      
    }
  
  return 0;
}