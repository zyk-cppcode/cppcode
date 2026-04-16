#pragma once
#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include "../server/Buffer.hpp"
#include "../server/logger.hpp"
//#include <muduo/net/http/HttpRequest.h>
//#include <muduo/net/http/HttpResponse.h>

// HTTP 通用工具类
class HttpUtil
{
public:
    // ------------------------------ 1. 字符串工具 ------------------------------
    // 去除字符串首尾空格、制表符、回车、换行
    static std::string trim(const std::string& s);
    
    // 忽略大小写比较字符串
    static bool iequals(const std::string& a, const std::string& b);

    // 分割字符串（sep:分隔符）
    static std::vector<std::string> split(const std::string& s, char sep);

    // ------------------------------ 2. URL 编解码 ------------------------------
    // URL 解码（处理 %xx 格式）
    static std::string urlDecode(const std::string& in);

    // URL 编码
    static std::string urlEncode(const std::string& in);

    // ------------------------------ 3. HTTP 协议工具 ------------------------------
    // 获取 HTTP 状态码描述（200->OK,404->Not Found）
    static std::string getStatusCodeDesc(int code);

    // 根据文件后缀获取 MIME 类型（.html->text/html）
    static std::string getMimeType(const std::string& path);

    // 判断是否长连接
    //static bool isKeepAlive(const muduo::net::HttpRequest& req);

    // ------------------------------ 4. 路径安全工具 ------------------------------
    // 规范化路径（消除 ../ ./ 等）
    static bool normalizePath(std::string* path);

    // 校验路径是否合法（防止目录穿越）
    static bool isValidPath(const std::string& path);

    // ------------------------------ 5. 文件系统工具 ------------------------------
    // 文件是否存在
    static bool fileExists(const std::string& path);

    // 是否是普通文件
    static bool isRegularFile(const std::string& path);

    // 是否是目录
    static bool isDirectory(const std::string& path);

    // 获取文件大小
    static off_t getFileSize(const std::string& path);

    // 获取文件最后修改时间
    static time_t getFileLastModify(const std::string& path);

    // 读取文件内容到 Buffer
    static bool readFileToBuffer(const std::string& path, Buffer* buf);
    //读取到 string
    static bool readFile(const std::string& path, std::string* content);

    // ------------------------------ 6. 响应构造工具 ------------------------------
    // 快速构造错误响应
    //static void makeErrorResponse(int code, const std::string& msg, muduo::net::HttpResponse* resp);
};