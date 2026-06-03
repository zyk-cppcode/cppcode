#include "HttpResponse.hpp"

int HttpResponse::GetStatusCode() const {
    return _status;
}
std::string HttpResponse::GetHeader(const std::string &key) const {
    auto it = _headers.find(key);
    if (it == _headers.end()) {
        return "";
    }
       return it->second;
}
bool HttpResponse::HasHeader(const std::string &key)const {
    return _headers.find(key) != _headers.end();
}
const std::unordered_map<std::string, std::string>& HttpResponse::GetHeaders() const {
    return _headers;
}

// const std::string& HttpResponse::GetBody() const {
//     return _body;
// }
std::string& HttpResponse::GetBody()  {
    return _body;
}

void HttpResponse::SetContent(const std::string &body,  const std::string &type = "text/html") {
    _body = body;
    SetHeader("Content-Type", type);
}
void HttpResponse::SetHeader(const std::string &key, const std::string &value) {
    _headers[key] = value;
}

//判断是否是短链接
bool HttpResponse::Close() {
   if (HasHeader("Connection") && GetHeader("Connection") == "close") {
        return true;  // 短连接，关闭
    }
    return false; // 长连接，不关闭
}
