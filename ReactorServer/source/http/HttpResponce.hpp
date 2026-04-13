#pragma once
#include <unordered_map>
#include <string>
class HttpResponce{
    public:
    int GetStatusCode() const;
    const std::unordered_map<std::string, std::string>& GetHeaders() const;
    const std::string& GetBody() const;
    private:
    int _status;// 响应状态码
    std::string _version;// HTTP版本
    std::unordered_map<std::string, std::string> _headers;// 响应头部
    std::string _body;// 响应体

} ;