#include "HttpResponce.hpp"

int HttpResponce::GetStatusCode() const {
    return _status;
}

const std::unordered_map<std::string, std::string>& HttpResponce::GetHeaders() const {
    return _headers;
}

const std::string& HttpResponce::GetBody() const {
    return _body;
}