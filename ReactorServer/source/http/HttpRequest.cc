#include "HttpRequest.hpp"

void HttpRequest::SetMethod(HttpMethod method) { _method = method; }

void HttpRequest::SetUrl(const std::string& url) { _url = url; }

void HttpRequest::SetPath(const std::string &path) { _path = path; }

void HttpRequest::SetQuery(const std::string& key, const std::string& value) 
{
   _query[key] = value; 
}


void HttpRequest::SetHeader(const std::string &key, const std::string &value)
{
  _headers[key] = value;
}
void HttpRequest::SetVersion(const std::string& version) {
    _version = version;
}
void HttpRequest::SetBody(const std::string &body) { _body = body; }

std::string HttpRequest::GetMethod() const { return HttpMethodtoString(_method); }
const std::string &HttpRequest::GetUrl() const { return _url; }

const std::string &HttpRequest::GetPath() const { return _path; }

const std::unordered_map<std::string, std::string> &HttpRequest::GetHeaders() const 
{
  return _headers;
}
//获取指定头部字段的值
std::string HttpRequest::GetHeader(const std::string &key) const {
    auto it = _headers.find(key);
    if (it == _headers.end()) {
        return "";
    }
       return it->second;
}
const std::string &HttpRequest::GetBody() const { return _body; }

//判断是否存在指定头部字段
bool HttpRequest::HasHeader(const std::string &key)const {
    return _headers.find(key) != _headers.end();
}
void HttpRequest::Reset() {
    _method = HttpMethod::UNKNOWN;
    _url.clear();
    _path.clear();
    _version="HTTP/1.1";
    _query.clear();
    _headers.clear();
    _body.clear();
}

//判断是否是短链接
bool HttpRequest::Close() {
    // 没有Connection字段，或者有Connection但是值是close，则都是短链接，否则就是长连接
    if (HasHeader("Connection") == true && GetHeader("Connection") == "keep-alive") {
        return false;
    }
    return true;
}