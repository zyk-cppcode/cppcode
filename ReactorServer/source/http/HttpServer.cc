#include "HttpServer.hpp"
#include "HttpContext.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "HttpUtil.hpp"
#include <iostream>

HttpServer::HttpServer(int port, int timeout):_server(port)
 {
    _server.SetTimeout(timeout);
    _server.SetConnectedCallback(std::bind(&HttpServer::onConnected,this,std::placeholders::_1));
    _server.SetMessageCallback(std::bind(&HttpServer::onMessage,this,std::placeholders::_1,std::placeholders::_2));
}
    void HttpServer::SetTimeout(int timeout){
    _server.SetTimeout(timeout);
    }

void HttpServer::SetBaseDir(const std::string &path) {
    _basedir = path;
}

void HttpServer::Get(const std::string &path, Handler handler) {
    _get_route[path] = handler;
}

void HttpServer::Post(const std::string &path, Handler handler) {
    _post_route[path] = handler;
}

void HttpServer::Put(const std::string &path, Handler handler) {
    _put_route[path] = handler;
}

void HttpServer::Delete(const std::string &path, Handler handler) {
    _delete_route[path] = handler;
}

void HttpServer::SetThreadNum(int num) {
    _server.SetThreadNum(num);
}

void HttpServer::SetEnableInactiveRelease(bool enable) {
    _server.SetEnableInactiveRelease(enable);
}

void HttpServer::Start() {
    _server.Start();
    
}
//将HttpResponse中的要素按照http协议格式进行组织，发送
void HttpServer::WriteReponse(const PtrConnection &conn,
                              const HttpRequest &request, 
                              HttpResponse &response){
    //补充head
    //长短链接
    response.SetHeader("Connection", request.GetHeader("Connection"));
    //长度
    const std::string& body = response.GetBody();
    if(!body.empty() && !response.HasHeader("Content-Length")) {
        response.SetHeader("Content-Length", std::to_string(body.size()));
    }
    if (response.GetBody().empty() == false && response.HasHeader("Content-Type") == false) {
        response.SetHeader("Content-Type", "application/octet-stream");
    }   
    if (response.GetRedirectFlag() == true) {
        response.SetHeader("Location", response.GetRedirectUrl());
    }
    std::string version = request.GetVersion();

    // 去掉版本里的换行、回车
    version.erase(remove_if(version.begin(), version.end(), [](char c) {
    return c == '\n' || c == '\r';
    }), version.end());
    std::string response_str;
    response_str=version+" "+std::to_string(response.GetStatusCode())+" "+HttpUtil::getStatusCodeDesc(response.GetStatusCode())+"\r\n";
    for(const auto &header:response.GetHeaders()){
        response_str+=header.first+": "+header. second+"\r\n";
    }     
    response_str += "\r\n";
    response_str += response.GetBody();
    conn->Send(response_str.data(),response_str.size());
}
bool HttpServer::IsFileHandler(HttpRequest &request,HttpResponse &response){
    if(_basedir.empty())
    {
        return false;//没有设置静态资源目录
    }
    if(request.GetMethod()!="GET"&&request.GetMethod()!="HEAD")
    {
        return false;//只处理GET和HEAD请求
    }
    std::string path=request.GetPath();
    //路径安全检查
    if(HttpUtil::isValidPath(path)==false)
    {
        response.SetStatus(400);
        response.SetBody("<h1>400 Bad Request</h1>");
        return true;
    }
    path=_basedir+request.GetPath();
    if(path.back()== '/')
    {
        path+="index.html";
    }
    //路径是否存在
    if(!HttpUtil::fileExists(path))
    {
        // response.SetStatus(404);
        // response.SetBody("<h1>404 Not Found</h1>");
        return false;
    }
    //LOG(LogLevel::DEBUG)<<"Static file request: "<<path;
    request.SetPath(path);//更新请求路径

    return true;
}
//静态资源的请求处理
bool HttpServer::FileHandler(HttpRequest &request,HttpResponse &response){
    std::string path=request.GetPath();
    std::string content;

    if(HttpUtil::readFile(path, &response.GetBody()))
    {
        response.SetStatus(200);
    }
    std::string type=HttpUtil::getMimeType(path);
    response.SetHeader("Content-Type", type);
    return true;
}
//功能性请求的分类处理
void HttpServer::Dispatcher(HttpRequest &request, HttpResponse &response,
                std::unordered_map<std::string, Handler> &route_map){
    // 获取请求路径
    std::string path = request.GetPath();
    // 在路由表里查找
    auto it = route_map.find(path);
    if (it != route_map.end()) {
        // 找到 → 执行处理函数
        it->second(request, response);
    } else {
        // 找不到 → 返回 404
        response.SetStatus(404);
        response.SetBody("<h1>404 Not Found</h1>");
    }
}

void HttpServer::ErrorHander(HttpRequest &request,HttpResponse &response){
    response.SetStatus(404);
    std::string body;
    LOG(LogLevel::DEBUG)<<"Error: "<<request.GetPath()<<" not found";
    response.SetContent(body,"text/html");
}
void HttpServer::Route(HttpRequest &request,HttpResponse &response){
    //注：所有比较都忽略大小写
    if(IsFileHandler(request,response))
    {
        FileHandler(request, response);//处理静态资源
        return;
    }
    if(HttpUtil::iequals(request.GetMethod(),"GET")||HttpUtil::iequals(request.GetMethod(),"HEAD"))
    {
        Dispatcher(request, response, _get_route);
    }
    else if (HttpUtil::iequals(request.GetMethod(),"POST"))
    {
        Dispatcher(request, response, _post_route); 
    }
    else if (HttpUtil::iequals(request.GetMethod(),"PUT"))
    {
        Dispatcher(request, response, _put_route);
    }
    else if (HttpUtil::iequals(request.GetMethod(),"DELETE"))
    {
        Dispatcher(request, response, _delete_route);
    }
    else
    {
     response.SetStatus(405);
        response.SetBody("<h1>405 Method Not Allowed</h1>");
    }
}
//
void HttpServer::onConnected(const PtrConnection &conn){
if (conn->Connected())
  {
    conn->SetContext(HttpContext());
  }
}
//
void HttpServer::onMessage(const PtrConnection &conn, Buffer *buffer) {
    LOG(LogLevel::DEBUG)<<"onMessage";
    //获取上下文，进行消息处理
    HttpContext *context=conn->GetContext()->get<HttpContext>();
    //解析请求
    context->ParseRequest(buffer);
    //HttpResponse responce(context->GetResponseStatu());
    HttpResponse response(200);

    HttpRequest& request=context->GetRequest();
    if(context->GetResponseStatu()>400)
    {
    ErrorHander(request,response);
    WriteReponse(conn,request,response);//直接返回错误响应
    buffer->moveReadOffset(buffer->getReadableSize());//丢弃请求数据
    context->Reset(); //重置上下文
    conn->Shutdown();
    return;
    }
    if(context->GetState()!=kGotAll){
        return; //请求未完整，等待更多数据
    }
    else
    { 
        //路由分发
        Route(request,response);
        //将响应发送给客户端
        WriteReponse(conn,request,response);
        //重置上下文，准备处理下一个请求
        context->Reset(); 
        //如果不是长链接，关闭连接
        if(response.Close()==true)
        {
            conn->Shutdown();
        }
    }
}

// void HttpServer::onMessage(const PtrConnection &conn, Buffer *buffer) {
//     // 只清空缓冲区，不解析、不路由、不回复
//     buffer->clear();
//     HttpContext *context=conn->GetContext()->get<HttpContext>();
//     // 构造一个最简单的响应
//     HttpResponse resp(200);
//     resp.SetBody("ok");
//     resp.SetHeader("Content-Length", "2");
//     WriteReponse(conn, context->GetRequest(), resp);
//     context->Reset();
// }