#pragma once

#include "HttpRequest.hpp"
#include "../server/Buffer.hpp"
#include "HttpUtil.hpp"
#include <string>
#include <vector>
typedef enum {
    kParseRequestLine, // 0：等待解析 请求行（GET / HTTP/1.1）
    kExpectHeaders,       // 1：等待解析 请求头（Host: xxx ...）
    kExpectBody,          // 2：等待解析 请求体（POST 才用）
    kGotAll               // 3：解析完成，拿到完整请求
} HttpRequestParseState;
#define MaxLineSize 8192
class HttpContext{
    public:
    HttpContext() ;
    const HttpRequest& getRequest() const ;
    bool parseRequest(Buffer* buf);
    bool gotAll() const;
    void reset();//
    HttpRequest& getRequest();
    private:
    bool parseRequestLine(Buffer *buf);
    bool parseHeaders(Buffer *bufx);
    bool parseBody(Buffer *buf);
    private:
    int _resp_statu; //响应状态码
    HttpRequestParseState _state;  // 解析状态
    HttpRequest _request;          // 解析出来的请求
};