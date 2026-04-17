/*超时连接测试1：创建一个客户端，给服务器发送一次数据后,查看服务器是否会正常的超时关闭连接*/

#include "../../server/Socket.hpp"
#include "../../server/logger.hpp"
#include <unistd.h>
#include <assert.h>

int main()
{
    Socket cli_sock;
    cli_sock.CreateClient(8111, "127.0.0.1");
    std::string req = "GET /hello HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 0\r\n\r\n";
    while(1) {
        std::cout<<"发送请求..."<<std::endl;
        assert(cli_sock.Send(req.c_str(), req.size()) != -1);
        char buf[1024] = {0};
        assert(cli_sock.Recv(buf, 1023));
        std::cout<<buf<<std::endl<<std::endl;
        sleep(15);
    }
    cli_sock.Close();
    return 0;
}