#include "../server/Buffer.hpp"
#include "HttpContext.hpp"
#include "HttpUtil.hpp"

#include <string>
#include <vector>
class HttpCodec {
private:
  bool parseRequestLine(Buffer *buf, HttpContext *ctx);
  bool parseHeaders(Buffer *buf, HttpContext *ctx);
  bool parseBody(Buffer *buf, HttpContext *ctx);

public:
};