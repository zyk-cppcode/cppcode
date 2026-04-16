#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include "../server/TcpServer.hpp"
#include "../server/Connection.hpp"
#include "../server/logger.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
using Handler = std::function<void(const HttpRequest &, HttpResponse &)>;
class HttpServer{
    public:
    HttpServer(int port,int timeout=60);
    void SetBaseDir(const std::string &path);//设置静态资源根目录
    std::string GetBaseDir() const { return _basedir; }
    
    //注册请求的处理函数
    void Get(const std::string &path, Handler handler);
    void Post(const std::string &path, Handler handler);  
    void Put(const std::string &path, Handler handler);    
    void Delete(const std::string &path, Handler handler);  
    void SetThreadNum(int num);//设置线程数量
    void SetEnableInactiveRelease(bool enable=true);//是否启用非活动连接释放
    void Start(); //
    private:
    void WriteReponse(const PtrConnection &conn,const HttpRequest &request, HttpResponse &responce);//将HttpResponse中的要素按照http协议格式进行组织，发送
    bool IsFileHandler(HttpRequest &request,HttpResponse &responce);//判断请求是否是静态资源
    bool FileHandler(HttpRequest &request,HttpResponse &responce);//静态资源的请求处理
    void Dispatcher(HttpRequest &request, HttpResponse &response,std::unordered_map<std::string, Handler> &route_map);//功能性请求的分类处理
    void Route(HttpRequest &request,HttpResponse &responce);//路由分发
    void onConnected(const PtrConnection &conn);
    void onMessage(const PtrConnection &conn, Buffer *buffer);
    void ErrorHander(HttpRequest &request,HttpResponse &responce);//错误处理
    private:
    std::unordered_map<std::string, Handler> _get_route;
    std::unordered_map<std::string, Handler> _post_route;
    std::unordered_map<std::string, Handler> _put_route;
    std::unordered_map<std::string, Handler> _delete_route;
    std::string _basedir;//静态资源根目录
    TcpServer _server;
};