/*大文件传输测试，给服务器上传一个大文件，服务器将文件保存下来，观察处理结果*/
/*
    上传的文件，和服务器保存的文件一致
*/
#include "../../server/Socket.hpp"
#include "../../server/logger.hpp"
#include "../HttpUtil.hpp"
#include <unistd.h>
#include <string.h>
#include <assert.h>

int main()
{
    EnableConsoleLogStrategy();

    Socket cli_sock;
    cli_sock.CreateClient(8111, "127.0.0.1");
    std::string req = "PUT /big HTTP/1.1\r\nConnection: keep-alive\r\n";
    std::string body;
    HttpUtil::readFile("./big.txt", &body);
     LOG(LogLevel::DEBUG)<<"1";
    req += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
   ssize_t  ret=cli_sock.Send(req.c_str(), req.size());
   ssize_t  ret2=cli_sock.Send(body.c_str(), body.size());

     LOG(LogLevel::DEBUG)<<ret;
        LOG(LogLevel::DEBUG)<<ret2;
    char buf[1024] = {0};
     LOG(LogLevel::DEBUG)<<"1";
   ret = cli_sock.Recv(buf, 1023);
    // if (ret <= 0) {
    //      std::cerr << "Recv failed, ret=" << ret << ", errno=" << strerror(errno) << std::endl;
    //     return 0;
    // }
     LOG(LogLevel::DEBUG)<<ret;
    std::cout<<buf<<std::endl<<std::endl;
    sleep(3);
    cli_sock.Close();
    return 0;
}