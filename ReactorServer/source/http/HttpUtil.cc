#include "HttpUtil.hpp"
#include <cstddef>

// ------------------------------ 1. 字符串工具 ------------------------------
// 去除字符串首尾空格、制表符、回车、换行
std::string HttpUtil::trim(const std::string &s) {
  size_t start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos)
    return "";
  size_t end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

// 忽略大小写比较字符串
bool HttpUtil::iequals(const std::string &a, const std::string &b) {
  if (a.size() != b.size())
    return false;
  for (size_t i = 0; i < a.size(); ++i) {
    if (tolower(a[i]) != tolower(b[i]))
      return false;
  }
  return true;
}

// 分割字符串（sep:分隔符）
std::vector<std::string> HttpUtil::split(const std::string &s, char sep) {
  std::vector<std::string> strs;
  std::string str = s;
  size_t pos = 0;
  while (str.size()) {
    pos = str.find(sep);
    if (pos == std::string::npos) {
      strs.push_back(str);
      break;
    } else {
      strs.push_back(std::string(str, 0, pos));
      str.erase(0, pos + 1);
    }
  }
  return strs;
}

// ------------------------------ 2. URL 编解码 ------------------------------
// char转两位 16 进制
std::string char_to_hex(char c) {
  std::string s;
  int i = static_cast<unsigned char>(c);
  int high = i / 16;
  int low = i % 16;
  // 把数字 0~15 转成 '0'-'F'
  auto to_char = [](int x) { return x < 10 ? (x + '0') : (x - 10 + 'A'); };
  s += to_char(high);
  s += to_char(low);
  return s; // 返回 "XX"
}
// 16 进制转 char
int hex_to_char(const std::string &hex) {
  int val = 0;
  for (char c : hex) {
    val <<= 4; // 等价于 val *= 16
    c = toupper(c);

    if (c >= '0' && c <= '9') {
      val += c - '0'; // 数字 0-9
    } else if (c >= 'A' && c <= 'F') {
      val += 10 + (c - 'A'); // 字母 A-F
    }
  }
  return val;
}

// URL 解码（处理 %xx 格式）
std::string HttpUtil::urlDecode(const std::string &in) {
  std::string out;
  int len = in.size();
  for (int i = 0; i < len; i++) {
    // 遇到 %，说明后面两位是十六进制，需要解码
    if (in[i] == '%' && i + 2 < len) {
      // 取出后面两位
      std::string hex = in.substr(i + 1, 2);
      // 转回字符
      out += (char)hex_to_char(hex);
      i += 2; // 跳过已处理
    } else {
      out += in[i];
    }
  }
  return out;
}

// URL 编码
std::string HttpUtil::urlEncode(const std::string &in) { 
  std::string out;
  for (auto c : in) {
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') || c == '-' || c == '_' || c == '.' ||
        c == '~') {
      out += c;
    } else {
      out += '%';
      out += char_to_hex(c);
    }
  }
  return out;
}

// ------------------------------ 3. HTTP 协议工具------------------------------
// 获取 HTTP 状态码描述（200->OK,404->Not Found）
std::string HttpUtil::getStatusCodeDesc(int code) {
  switch (code) {
  case 100: return "Continue";
  case 101: return "Switching Protocols";
  case 200: return "OK";
  case 201: return "Created";
  case 202: return "Accepted";
  case 204: return "No Content";
  case 206: return "Partial Content";
  case 301: return "Moved Permanently";
  case 302: return "Found";
  case 303: return "See Other";
  case 304: return "Not Modified";  
  case 307: return "Temporary Redirect";
  case 308: return "Permanent Redirect";
  case 400: return "Bad Request";
  case 401: return "Unauthorized";
  case 403: return "Forbidden";
  case 404: return "Not Found";
  case 405: return "Method Not Allowed";
  case 406: return "Not Acceptable";
  case 408: return "Request Timeout";
  case 409: return "Conflict";
  case 411: return "Length Required";
  case 413: return "Payload Too Large";
  case 414: return "URI Too Long";
  case 415: return "Unsupported Media Type";
  case 429: return "Too Many Requests";
  case 500: return "Internal Server Error";
  case 501: return "Not Implemented";
  case 502: return "Bad Gateway";
  case 503: return "Service Unavailable";
  case 504: return "Gateway Timeout";
  case 505: return "HTTP Version Not Supported";
  default: return "Unknown Status";
  }
}

// 根据文件后缀获取 MIME 类型（.html->text/html）
std::string HttpUtil::getMimeType(const std::string &path) {
  size_t pos = 0;
  pos = path.rfind('.');
  if (pos == std::string::npos) {
    LOG(LogLevel::ERROR) << "PATH is not valid";
    return "application/octet-stream";
  }
  // 截取后缀
  std::string ext(std::string(path, pos + 1, path.size()));
  // 转小写
  for (auto &c : ext) {
    c = tolower(c);
  }
  if (ext == "html" || ext == "htm")
    return "text/html";
  if (ext == "css")
    return "text/css";
  if (ext == "js")
    return "application/javascript";
  if (ext == "jpg" || ext == "jpeg")
    return "image/jpeg";
  if (ext == "png")
    return "image/png";
  if (ext == "gif")
    return "image/gif";
  if (ext == "bmp")
    return "image/bmp";
  if (ext == "ico")
    return "image/x-icon";
  if (ext == "txt")
    return "text/plain";
  if (ext == "mp3")
    return "audio/mpeg";
  if (ext == "mp4")
    return "video/mp4";
  if (ext == "json")
    return "application/json";
  if (ext == "pdf")
    return "application/pdf";
  if (ext == "zip")
    return "application/zip";
  if (ext == "gz")
    return "application/gzip";
  // 默认二进制流
  return "application/octet-stream";
}

// 判断是否长连接
// bool HttpUtil::isKeepAlive(const muduo::net::HttpRequest& req){}

// ------------------------------ 4. 路径安全工具 ------------------------------
// 规范化路径（消除 ../ ./ 等）
bool HttpUtil::normalizePath(std::string *path) {
  std::vector<std::string> spl=split(*path, '/');
  std::vector<std::string> outpath;
  for(auto c:spl)
  {
    
    if((c ==".")||(c.empty()))
    {
        continue;
    }
    else
    {
      if(c=="..")
      {
        if(outpath.empty())
          return false;
        else
        {
        outpath.pop_back();
        }
      }
      else
      {
        outpath.push_back(c);
      }
    }

  }
  path->clear();
  for(auto c:outpath)
  {
    *path+='/';
    *path+=c;
  }
  if (path->empty()) {
    *path = "/";
  }
  return true;
}

// 校验路径是否合法（防止目录穿越）
bool HttpUtil::isValidPath(const std::string &path) {
  std::string p=path;
  if (!normalizePath(&p)) {
        return false;
    }
   return !p.empty() && p[0] == '/';
}

// ------------------------------ 5. 文件系统工具 ------------------------------
// 文件是否存在
bool HttpUtil::fileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

// 是否是普通文件
bool HttpUtil::isRegularFile(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
    return false;
  return S_ISREG(st.st_mode);
}

// 是否是目录
bool HttpUtil::isDirectory(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
    return false;
  return S_ISDIR(st.st_mode);
}

// 获取文件大小
off_t HttpUtil::getFileSize(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
    return -1;
  return st.st_size;
}

// 获取文件最后修改时间
time_t HttpUtil::getFileLastModify(const std::string &path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0)
    return -1;
  return st.st_mtime;
}

// 读取文件内容到 Buffer
bool HttpUtil::readFileToBuffer(const std::string &path, Buffer *buf) {
  // 检查文件是否存在
  if (!fileExists(path)) {
    return false;
  }
  // 判断是否是普通文件
  if (!isRegularFile(path)) {
    return false;
  }
  // 打开文件
  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return false;
  }
  off_t fileSize = getFileSize(path);
  buf->ensureWriteSize(fileSize);
  size_t readsize = read(fd, buf->getWriteOffset(), fileSize);
  
  if (readsize == (size_t)-1) {
    close(fd);
    return false;
  }
  buf->moveWriteOffset(readsize);
  close(fd);
  return true;
}
    //读取到 string
bool HttpUtil::readFile(const std::string& path, std::string* content){
  // 检查文件是否存在
  if (!fileExists(path)) {
    return false;
  }
  // 判断是否是普通文件
  if (!isRegularFile(path)) {
    return false;
  }
  // 打开文件
  int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return false;
  }
  off_t fileSize = getFileSize(path);
  content->resize(fileSize);
  size_t readsize = read(fd, content->data(), fileSize);
  
  if (readsize == (size_t)-1) {
    close(fd);
    return false;
  }
  close(fd);
  return true;
}
// ------------------------------ 6. 响应构造工具 ------------------------------
// 快速构造错误响应
// void HttpUtil::makeErrorResponse(int code, const std::string& msg,
// muduo::net::HttpResponse* resp){} 
