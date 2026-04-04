 #include "Acceptor.hpp"

 Acceptor::Acceptor(EventLoop *loop, uint16_t port):_loop((loop))
 {
  _socket.CreateServer(port);
  _accept_channel = std::make_unique<Channel>(loop, _socket.Fd());
  _accept_channel->SetReadCallBack(std::bind(&Acceptor::HandleRead, this));
  _accept_channel->EnableRead();
 }
    Acceptor::~Acceptor(){}
    void Acceptor::SetAcceptCallBack(const AcceptCallBack &cb){
        _accept_callback = cb;
    }

void Acceptor::HandleRead(){
    int newfd = _socket.Accept();
    if(newfd < 0){
        LOG(LogLevel::ERROR) << "accept error";
        return;
    }
     LOG(LogLevel::INFO) << "accept success, newfd=" << newfd;
     if(newfd >= 0 && _accept_callback){
         _accept_callback(newfd);
     }
}