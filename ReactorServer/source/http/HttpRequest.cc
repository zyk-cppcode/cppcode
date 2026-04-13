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

HttpMethod HttpRequest::GetMethod() const { return _method; }
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

void HttpRequest::Reset() {
    _method = HttpMethod::UNKNOWN;
    _url.clear();
    _path.clear();
    _version.clear();
    _query.clear();
    _headers.clear();
    _body.clear();
}