#pragma once
#include <unordered_map>
#include <string>
#include "../server/logger.hpp"
class HttpResponse{
    public:
    HttpResponse(int statu=200):_status(statu){}; // 构造函数，默认状态码为200
    int GetStatusCode() const;
    std::string GetHeader(const std::string &key) const; // 获取指定头部字段的值
    bool GetRedirectFlag() const { return _redirect_flag; } // 获取重定向标志
    std::string GetRedirectUrl() const { return _redirect_url; } // 获取重定向URL
    bool HasHeader(const std::string &key) const;// 判断是否存在指定头部字段

    const std::unordered_map<std::string, std::string>& GetHeaders() const;
    //const std::string& GetBody() const;
    std::string& GetBody() ;

    void SetStatus(int statu){_status=statu;}
    void SetContent(const std::string &body,  const std::string &type);
    void SetHeader(const std::string &key, const std::string &value) ;
    void SetBody(std::string body){_body=body;}
    bool Close();
    private:
    int _status;// 响应状态码
    std::string _version;// HTTP版本
    bool _redirect_flag = false;// 是否是重定向
    std::string _redirect_url;// 重定向URL
    std::unordered_map<std::string, std::string> _headers;// 响应头部
    std::string _body;// 响应体

} ;