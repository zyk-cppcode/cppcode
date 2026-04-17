#include "Connection.hpp"
#include "logger.hpp"
#include "Buffer.hpp"
#include "Channel.hpp"
#include "EventLoop.hpp"
#include "Socket.hpp"
#include <algorithm>
#include <cerrno>
#include <sys/types.h>


// 描述符可读事件触发后调用的函数，接收socket数据放到接收缓冲区中，然后调用_message_call
void Connection::HandleRead() {
  // 接收socket数据放到接收缓冲区中
  char buf[65535];
  int ret = _socket->Recv(buf, sizeof(buf));
  //std::cout<<"buf:"<<buf<<std::endl;
  if (ret < 0) {
    int err = errno; // 保存 errno
    if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR) {
      return;
    }
    LOG(LogLevel::ERROR) << "Recv failed, errno=" << err<<"fd:"<<_sockfd;
    ShutdownInLoop();
    return;
  } else if (ret == 0) {
    // 对端关闭连接，必须关闭
    LOG(LogLevel::DEBUG) << "Peer closed connection";
    ShutdownInLoop();
    return;
  }
  _in_buffer->write(buf, ret);
  //std::cout<<_in_buffer->getReadableSize()<<std::endl;
  // 调用_message_callback
  if (_in_buffer->getReadableSize() > 0) {
   _message_callback(shared_from_this(), _in_buffer);
  //std::cout<<_in_buffer->getReadableSize()<<std::endl;
  }
}
// 描述符可写事件触发后调用的函数,将发送缓冲区中的数据进行发送
void Connection::Handlewrite() {
  // 发送缓冲区是否有数据
  int size = _out_buffer->getReadableSize();
  if (size == 0) {
    LOG(LogLevel::DEBUG) << "OutBuffer is empty!";
    _channel->DisableWrite();
    // 链接处于待关闭状态，且缓冲区无数据
    if (_statu == DISCONNECTING) {
      // 释放
      ReleaseInLoop();
    }
    return;
  }
  // 将发送缓冲区中的数据进行发送
  ssize_t ret = _socket->Send(_out_buffer->getReadOffset(), size);
  if (ret < 0) {
    // 防止 errno 被修改，保存
    int err = errno;
    if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR) {
      return;
    }
    // 发送失败
    else {
      // 出错，释放链接
      LOG(LogLevel::ERROR) << "Send failed, errno=" << err;
      ReleaseInLoop();
      return;
    }
  }
  // 发送成功，移动发送缓冲区的读偏移
  _out_buffer->moveReadOffset(ret);
  // 如果发送缓冲区没有数据了，取消写事件的监控
  if (_out_buffer->getReadableSize() == 0) {
    _channel->DisableWrite();
    // 链接处于待关闭状态，且缓冲区无数据
    if (_statu == DISCONNECTING) {
      // 释放链接
      ReleaseInLoop();
    }
  }
}
// 描述符触发挂断事件
void Connection::Handleclose() {
     if (_in_buffer->getReadableSize() > 0) {
    return _message_callback(shared_from_this(), _in_buffer);
  }
  ShutdownInLoop();
}
// 描述符触发出错事件
void Connection::HandleError() {
    Handleclose();
}
// 描述符触发任意事件
void Connection::HandleEvent() {
    if(_enable_inactive_release==true)
    {// 刷新活跃度
        _loop->TimerRefresh(_conn_id);
    }
    //调用用户设置的事件回调
    if(_event_callback)
    _event_callback(shared_from_this());
}
// 连接获取之后，所处的状态下要进行各种设置(启动读监控)
void Connection::EstablishedInLoop() {
    //设置状态已启动
    _statu=CONNECTED;
    //启动读监控
    _channel->EnableRead();
    //执行回调
    if(_connected_callback)
    _connected_callback(shared_from_this());

}
// 实际的释放接口
void Connection::ReleaseInLoop() {
    //防止重复释放
    if (_statu == DISCONNECTED) {return;}
    //修改状态
    _statu=DISCONNECTED;
    //移除事件监控
    _channel->Remove();
    //关闭套接字
    _socket->Close();
    //在释放连接前，先取消连接的闲置超时定时器
    if(_loop->HasTimer(_conn_id)){CancelInactiveReleaseInLoop();}
    //执行用户设置回调
    if(_closed_callback){_closed_callback(shared_from_this());}
    //执行服务器关闭回调
    if(_server_closed_callback){_server_closed_callback(shared_from_this());}
}
void Connection::SendInLoop(char *data, size_t len) {
  //判断状态
  if (_statu != CONNECTED) {
    LOG(LogLevel::ERROR) << "Send failed: connection not connected!";
    return;
  }
  //把数据放入发送缓冲区
  _out_buffer->write(data, len);
  //监控写事件
  _channel->EnableWrite();
}
void Connection::ShutdownInLoop() {
  //检查，链接已关闭，返回
  if (_statu != CONNECTED) {
        return;
    }
    _statu = DISCONNECTING;
    // 取消写事件的监控，等发送缓冲区中的数据发送完后再释放连接
  if (_in_buffer->getReadableSize()>0)
  {//输入缓冲区不为空，执行用户设置回调
    if(_message_callback)
    {
      _message_callback(shared_from_this(), _in_buffer);
    }
  }
  //输出缓冲区不为空
  if(_out_buffer->getReadableSize()>0)
  {
  //启动可写事件
    _channel->EnableWrite();
  }
  else
   {//输出缓冲区为空，释放链接
    ReleaseInLoop();
   }
  }
  
void Connection::EnableInactiveReleaseInLoop(int sec) {
  _enable_inactive_release=true;
  //如果有定时任务，就刷新
  if(_loop->HasTimer(_conn_id))
  {
    _loop->TimerRefresh(_conn_id);
  }
  else
  {
    //如果没有，就添加
    _loop->TimerAdd(_conn_id,sec,std::bind(&Connection::ReleaseInLoop,this));
  }

}
void Connection::CancelInactiveReleaseInLoop() {
  _enable_inactive_release=false;
  //删除定时任务
  if(_loop->HasTimer(_conn_id))
  {
    _loop->TimerCancel(_conn_id);
  }

}
//更新协议
void Connection::UpgradeInLoop(const Any &context,
                               const ConnectedCallback &conn,
                               const MessageCallback &msg,
                               const ClosedCallback &closed,
                               const AnyEventCallback &event) {
  _context=&context;
  _connected_callback=conn;
  _message_callback=msg;
  _closed_callback=closed;
  _event_callback=event;
}

Connection::Connection(int conn_id, int sockfd, EventLoop *loop)
    : _conn_id(conn_id), _sockfd(sockfd),_loop(loop),
     _socket(new Socket(sockfd)),_channel(new Channel(loop, sockfd)), 
      _enable_inactive_release(false),_statu(CONNECTING) {
  // 设置Channel的回调函数
  _channel->SetReadCallBack(std::bind(&Connection::HandleRead, this));
  _channel->SetWriteCallBack(std::bind(&Connection::Handlewrite, this));
  _channel->SetCloseCallBack(std::bind(&Connection::Handleclose, this));
  _channel->SetErrorCallBack(std::bind(&Connection::HandleError, this));
  _channel->SetEventCallBack(std::bind(&Connection::HandleEvent, this));
  // 初始化输入输出缓冲区
  _in_buffer = new Buffer();
  _out_buffer = new Buffer();
}
Connection::~Connection() {
  LOG(LogLevel::DEBUG) << "Connection destructed, conn_id=" << _conn_id;
}
// 获取管理的文件描述符
int Connection::Fd() {
  return _sockfd;
}
// 获取连接ID
int Connection::Id() {
  return _conn_id;
}
// 是否处于CONNECTED状态
bool Connection::Connected() {
  return _statu == CONNECTED;
}
// 设置上下文--连接建立完成时进行调用
void Connection::SetContext(const Any &context) {
  _context=context;
}
// 获取上下文，返回指针
Any*  Connection::GetContext() {
  return &_context;
}
void Connection::SetConnectedCallback(const ConnectedCallback &cb) {
  _connected_callback = cb;
}
void Connection::SetMessageCallback(const MessageCallback &cb) {
  _message_callback = cb;
}
void Connection::SetClosedCallback(const ClosedCallback &cb) {
  _closed_callback = cb;
}
void Connection::SetEventCallback(const AnyEventCallback &cb) {
  _event_callback = cb;
}
void Connection::SetServerClosedCallback(const ClosedCallback &cb) {
  _server_closed_callback = cb;
}

//连接建立就绪后，进行channel回调设置,启动读监控,调用_connected_cal1back
void Connection::Established() {
_loop->RunInLoopThread(std::bind(&Connection::EstablishedInLoop, this));
}
void Connection::Send(char *data,size_t len) {
  // 实现发送数据的逻辑
  _loop->RunInLoopThread(std::bind(&Connection::SendInLoop, this, data, len));
}
void Connection::Shutdown() {
  // 实现关闭连接的逻辑
  _loop->RunInLoopThread(std::bind(&Connection::ShutdownInLoop, this));
}
void Connection::EnableInactiveRelease(int sec) {
  // 实现启动非活跃销毁的逻辑
  _loop->RunInLoopThread(std::bind(&Connection::EnableInactiveReleaseInLoop, shared_from_this(), sec));
}
void Connection::CancelInactiveRelease() {
  // 实现取消非活跃销毁的逻辑
  _loop->RunInLoopThread(std::bind(&Connection::CancelInactiveReleaseInLoop, shared_from_this()));
}
//切换协议---重置上下文以及阶段性处理函数
void Connection::Upgrade(const Any&context,
                         const ConnectedCallback &conn, 
                         const MessageCallback &msg,
                         const ClosedCallback &closed,
                         const AnyEventCallback &event) {
  // 实现切换协议的逻辑
  //切换协议涉及到修改连接的上下文以及回调函数，这些操作需要在EventLoop所属的线程中执行，以保证线程安全。
  _loop->AssertInLoopThread();
  _loop->RunInLoopThread(std::bind(&Connection::UpgradeInLoop,
                                       this, context, conn, msg, closed, event));
}