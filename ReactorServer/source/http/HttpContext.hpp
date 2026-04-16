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
    const HttpRequest& GetRequest() const ;//获取解析出来的请求
    bool ParseRequest(Buffer* buf);//解析请求，返回是否成功
    bool GotAll() const;//是否解析完成
    void Reset();//
    int GetResponseStatu() const { return _resp_statu; }
    void SetResponseStatu(int statu) { _resp_statu = statu; }
    HttpRequestParseState GetState() const;
    HttpRequest& GetRequest();//
    private:
    bool ParseRequestLine(Buffer *buf);//解析请求行
    bool ParseHeaders(Buffer *bufx);   //解析请求头
    bool ParseBody(Buffer *buf);       //解析请求体
    private:
    int _resp_statu = 200; //响应状态码
    HttpRequestParseState _state;  // 解析状态
    HttpRequest _request;          // 解析出来的请求
};