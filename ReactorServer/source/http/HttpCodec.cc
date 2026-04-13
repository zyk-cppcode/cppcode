#include "HttpCodec.hpp"
#define MaxLineSize 4096

bool HttpCodec::parseRequestLine(Buffer *buf, HttpContext *ctx) {
  // 获取一行数据
  std::string line = buf->GetLine();
  if (line.empty()) {
    return false; // 数据不足一行，等待更多数据
  }
  if (line.size() > MaxLineSize)
    return false; // 数据过长，错误
  // 获取数据成功，解析
  std::vector<std::string> parts =
      HttpUtil::split(line, ' '); // 按空格分割请求行
  if (parts.size() != 3) {
    return false; // 请求行格式错误
  }
  if (parts[0].empty() || parts[1].empty() || parts[2].empty()) {
    return false; // 请求行格式错误，方法、路径或版本不能为空
  }
  HttpMethod m = stringToHttpMethod(parts[0]);
  if (m == HttpMethod::UNKNOWN) {
    return false;
  }

  if (parts[2].compare(0, 5, "HTTP/") != 0) {
    return false; // HTTP版本格式错误，必须以 "HTTP/" 开头
  }
  // 解析 url
  size_t pos = parts[1].find('?');
  if (pos != std::string::npos) {
    std::string path = parts[1].substr(0, pos);
    if (HttpUtil::isValidPath(path) == false) {
      return false; // 请求路径不合法，可能存在目录穿越风险
    }
    ctx->getRequest().SetPath(path); // 设置路径
    // 解析查询参数
    std::string queryString = parts[1].substr(pos + 1);
    std::vector<std::string> params = HttpUtil::split(queryString, '&');
    for (const std::string &param : params) {
      size_t eqPos = param.find('=');
      if (eqPos != std::string::npos) {
        std::string key = param.substr(0, eqPos);    // key
        std::string value = param.substr(eqPos + 1); // value
        ctx->getRequest().SetQuery(key, value);      // 存储查询参数
      }
    }
  } else {
    ctx->getRequest().SetPath(parts[1]); // 没有查询参数，直接设置路径
  }
  ctx->getRequest().SetMethod(m);
  ctx->getRequest().SetUrl(parts[1]);
  ctx->getRequest().SetVersion(parts[2]);
  ctx->SetState(kExpectHeaders); // 进入解析请求头状态

  return true;
}

bool HttpCodec::parseHeaders(Buffer *buf, HttpContext *ctx) {
  while (true) {
    std::string line = buf->GetLine();
    if (line.empty()) {
      return false; // 数据不足一行，等待更多数据
    }
    // 2. 请求行不能是空行
    if (line == "\r\n" || line == "\n") {
      return false;
    }

    // 3. 去掉末尾的 \r\n
    if (!line.empty() && line.back() == '\n')
      line.pop_back();
    if (!line.empty() && line.back() == '\r')
      line.pop_back();
    if (line.size() > MaxLineSize)
      return false; // 数据过长，错误
    if (line == "\r\n" || line == "\n") {
      ctx->SetState(kExpectBody); // 进入解析请求体状态
      break;                      // 请求头解析完成
    }
    size_t colonPos = line.find(':');
    if (colonPos == std::string::npos) {
      return false; // 请求头格式错误，必须包含冒号
    }
    std::string key = line.substr(0, colonPos);
    std::string value = line.substr(colonPos + 1);
    HttpUtil::trim(key);   // 去除键的前后空白
    HttpUtil::trim(value); // 去除值的前后空白
    if (key.empty() || value.empty()) {
      return false; // 请求头格式错误，键或值不能为空
    }
    ctx->getRequest().SetHeader(key, value); // 存储请求头
  }
  return true;
}

bool HttpCodec::parseBody(Buffer *buf, HttpContext *ctx) {
  // 1. 获取 Content-Length 请求头，确定请求体长度
  std::string contentLengthStr = ctx->getRequest().GetHeader("Content-Length");
  if (contentLengthStr.empty()) {
    ctx->SetState(kGotAll);
    return true; // 没有请求体，解析完成
  }
  size_t Length = std::stoul(contentLengthStr);
  size_t need = Length - ctx->getRequest().GetBody().size();
  // 2. 解析请求体
  std::string chunk; ;
  if(buf->getReadableSize()>=need)
  {
    buf->read(chunk,need);
    std::string new_body = ctx->getRequest().GetBody() + chunk;
    ctx->getRequest().SetBody(new_body); // 存储请求体
    ctx->SetState(kGotAll); // 解析完成
    return true;
  }
  if(buf->getReadableSize()<need)
  { 
    buf->read(chunk,buf->getReadableSize()); // 读取当前可读数据
    std::string new_body = ctx->getRequest().GetBody() + chunk;
    ctx->getRequest().SetBody(new_body); // 存储请求体
    return false; // 数据不足，等待更多数据
  }
  return true;
}