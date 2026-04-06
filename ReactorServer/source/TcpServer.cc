#include "TcpServer.hpp"

TcpServer::TcpServer(int port, int thread_num)
    : _port(port),
      _thread_num(thread_num),
      _base_loop(), 
      _enable_inactive_release(false),
      _acceptor(nullptr), 
      _pool(nullptr) {
    _acceptor = new Acceptor(&_base_loop, _port);
    _acceptor->SetAcceptCallBack(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer() {
}

void TcpServer::SetMessageCallback(MessageCallback cb) {
    _message_callback = cb;
}

void TcpServer::SetConnectedCallback(ConnectedCallback cb) {
    _connected_callback = cb;
}

void TcpServer::SetClosedCallback(ClosedCallback cb) {
    _closed_callback = cb;
}

void TcpServer::SetEventCallback(AnyEventCallback cb) {
    _event_callback = cb;
}
void TcpServer::SetEnableInactiveRelease(bool enable){ 
    _enable_inactive_release = enable; 
}

void TcpServer::SetThreadNum(int thread_num){
    _thread_num = thread_num;
}
void TcpServer::Start(){}
void TcpServer::NewConnection(int sockfd){}
void TcpServer::RemoveConnection(const PtrConnection &conn){}
void TcpServer::RemoveConnectionInLoop(const PtrConnection& conn){}