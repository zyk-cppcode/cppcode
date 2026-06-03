/*给服务器发送一个数据，告诉服务器要发送1024字节的数据，但是实际发送的数据不足1024，查看服务器处理结果*/
/*
    1. 如果数据只发送一次，服务器将得不到完整请求，就不会进行业务处理，客户端也就得不到响应，最终超时关闭连接
    2. 连着给服务器发送了多次 小的请求，  服务器会将后边的请求当作前边请求的正文进行处理，而后便处理的时候有可能就会因为处理错误而关闭连接
*/

#include "../../server/Socket.hpp"
#include "../../server/logger.hpp"
#include <unistd.h>
#include <assert.h>

int main()
{
    Socket cli_sock;
    cli_sock.CreateClient(8111, "127.0.0.1");
    std::string req = "GET /hello.txt HTTP/1.1\r\nConnection: keep-alive\r\nContent-Length: 100\r\n\r\nzyk";
    while(1) {
        ssize_t ret = cli_sock.Send(req.c_str(), req.size());
        ret = cli_sock.Send(req.c_str(), req.size());
        ret = cli_sock.Send(req.c_str(), req.size());
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