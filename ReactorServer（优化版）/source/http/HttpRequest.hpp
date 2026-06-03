#pragma once
#include <string>
#include <unordered_map>
//枚举HTTP方法
enum class HttpMethod {
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    PATCH,
    OPTIONS,
    UNKNOWN
};
inline std::string HttpMethodtoString(HttpMethod method){
    switch(method) {
        case HttpMethod::GET: return "GET";
        case HttpMethod::POST: return "POST";
        case HttpMethod::PUT: return "PUT";
        case HttpMethod::DELETE: return "DELETE";
        case HttpMethod::HEAD: return "HEAD";
        case HttpMethod::PATCH: return "PATCH";
        case HttpMethod::OPTIONS: return "OPTIONS";
        case HttpMethod::UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}


inline HttpMethod stringToHttpMethod(const std::string& methodStr) {
    if (methodStr == "GET")    return HttpMethod::GET;
    if (methodStr == "POST")   return HttpMethod::POST;
    if (methodStr == "HEAD")   return HttpMethod::HEAD;
    if (methodStr == "PUT")    return HttpMethod::PUT;
    if (methodStr == "DELETE") return HttpMethod::DELETE;
    if (methodStr == "OPTIONS")return HttpMethod::OPTIONS;
    if (methodStr == "PATCH")  return HttpMethod::PATCH;
    return HttpMethod::UNKNOWN;
}
class HttpRequest
    {public:
        //设置接口
        void SetMethod(HttpMethod method);
        void SetUrl(const std::string& url);
        void SetPath(const std::string& path);
        void SetVersion(const std::string& version);
        void SetQuery(const std::string& key, const std::string& value); // 设置查询参数
        void SetHeader(const std::string& key, const std::string& value); // 单个加请求头
        void SetBody(const std::string& body);

        //取值接口
        std::string GetMethod() const;
        const std::string& GetUrl() const;
        const std::string& GetPath() const;
        const std::string& GetVersion() const { return _version; }
        const std::unordered_map<std::string, std::string>& GetHeaders() const;
        //HttpMethod GetHeader(const std::string &key) const; // 获取指定头部字段的值
        std::string GetHeader(const std::string &key) const; // 获取指定头部字段的值

        const std::string& GetBody() const;
        bool HasHeader(const std::string &key) const;// 判断是否存在指定头部字段
        void Reset(); // 重置请求
        bool Close(); // 判断是否是短链接
    private:
        HttpMethod _method;// 请求方法
        std::string _url;// 请求URL
        std::string _path;// 请求资源路径
        std::string _version;// HTTP版本
        std::unordered_map<std::string, std::string> _query ;// 查询参数
        std::unordered_map<std::string, std::string> _headers;// 请求头部
        std::string _body;// 请求体
};