#include "HttpCodec.hpp"
#include "HttpServer.hpp"
#include "HttpUtil.hpp"
#include "HttpContext.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <iostream>

std::string RequestStr(const HttpRequest &req) {
    std::stringstream ss;
    ss << req.GetMethod() << " " << req.GetPath() << " " << req.GetVersion() << "\r\n";

    for (auto &it : req.GetHeaders()) {
        ss << it.first << ": " << it.second << "\r\n";
    }
    ss << "\r\n";
    ss << req.GetBody();
    return ss.str();
}
void Hello(const HttpRequest &req, HttpResponse &rsp) 
{
    rsp.SetContent(RequestStr(req), "text/plain");
}
void Login(const HttpRequest &req, HttpResponse &rsp) 
{
    rsp.SetContent(RequestStr(req), "text/plain");
}
// void PutFile(const HttpRequest &req, HttpResponse *rsp) 
// {
//     std::string pathname = "./static" + req.GetPath();
//     Util::WriteFile(pathname, req.GetBody());
// }
void DelFile(const HttpRequest &req, HttpResponse &rsp) 
{
    rsp.SetContent(RequestStr(req), "text/plain");
}
int main()
{   
    EnableConsoleLogStrategy();
    LOG(LogLevel::DEBUG)<<"Starting HTTP Server on port 8111...";
    HttpServer server(8111);
    server.SetEnableInactiveRelease(false);
    server.SetThreadNum(2);
    server.SetBaseDir("./static");
    server.Get("/hello.txt", Hello);
    server.Post("/login", Login);
    //server.Put("/1234.txt", PutFile);
    server.Delete("/1234.txt", DelFile);
    // server.Put("/file", PutFile);
    // server.Delete("/file", DelFile);
    //LOG(LogLevel::DEBUG)<<"Base directory: "<<server.GetBaseDir();
    server.Start();
    return 0;
}