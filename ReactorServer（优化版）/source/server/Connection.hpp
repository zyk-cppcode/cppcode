#pragma once

#include "Channel.hpp"
#include <functional>
#include <memory>
#include "any.hpp"
#include "memorypool/Alloc.hpp"
class Socket;
class Buffer;
class Channel;
class Poller;
class Any;
class EventLoop;
class Connection;
// DISCONNECTED --连接关闭状态;
// CONNECTING --连接建立成功-待处理状态
// CONNECTED --连接建立完成，各种设置已完成，可以通信的状态;
// DISCONNECTING --待关闭状态
typedef enum { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING } ConnStatu;
using PtrConnection = std::shared_ptr<Connection>;
using ConnectedCallback = std::function<void(const PtrConnection &)>;
using MessageCallback = std::function<void(const PtrConnection &, Buffer *)>;
using ClosedCallback = std::function<void(const PtrConnection &)>;
using AnyEventCallback = std::function<void(const PtrConnection &)>;


class Connection :public std::enable_shared_from_this<Connection>{
private:
/*五个channel的事件回调函数*/
void HandleRead();//描述符可读事件触发后调用的函数，接收socket数据放到接收缓冲区中，然后调用_message_ca11
void Handlewrite();//描述符可写事件触发后调用的函数,将发送缓冲区中的数据进行发送
void Handleclose();//描述符触发挂断事件
void HandleError();//描述符触发出错事件
void HandleEvent();//描述符触发任意事件
void EstablishedInLoop();//连接获取之后，所处的状态下要进行各种设置(给channel设置事件回调，启动读监控)
void ReleaseInLoop();//这个接口才是实际的释放接口
void SendInLoop(char *data, size_t len);
void ShutdownInLoop();
void EnableInactiveReleaseInLoop(int sec);
void CancelInactiveReleaseInLoop();
void UpgradeInLoop(const Any &context,
                   const ConnectedCallback &conn,
                   const MessageCallback &msg,
                   const ClosedCallback &closed,
                   const AnyEventCallback &event);
public:
  Connection(int conn_id, int sockfd, EventLoop *loop);
  ~Connection();
  // 原始代码：未定义 operator new/delete，同时 ~Connection() 为空导致 _socket/_channel/_in_buffer/_out_buffer 内存泄漏
  // 优化后：使用 MemoryPool 的 ConcurrentAlloc/ConcurrentFree 替代全局默认分配器
  void* operator new(size_t size) { return ConcurrentAlloc(size); }
  void operator delete(void* ptr) { ConcurrentFree(ptr); }
  void handleEvent();
  void send(const char *data, size_t len);
  void SetConnectedCallback(const ConnectedCallback &cb);
  void SetMessageCallback(const MessageCallback &cb);
  void SetClosedCallback(const ClosedCallback &cb);
  void SetEventCallback(const AnyEventCallback &cb);
  void SetServerClosedCallback(const ClosedCallback &cb);
  int Fd();//获取管理的文件描述符
  int Id();//获取连接ID
  bool Connected();//是否处于CONNECTED状态
  void SetContext(const Any&context);//设置上下文--连接建立完成时进行调用
  Any *GetContext();//获取上下文，返回指针
//连接建立就绪后，进行channel回调设置,启动读监控,调用_connected_cal1back
void Established();
void Send(char *data,size_t len);//发送数据，将数据放到发送缓冲区，启动写事件监控
void Shutdown();//提供给组件使用者的关闭接口--并不实际关闭，需要判断有没有数据待处理
void EnableInactiveRelease(int sec);//启动非活跃销毁,并定义多长时间无通信就是非活跃,添加定时任务
void CancelInactiveRelease();//取消非活跃销毁//切换协议---重置上下文以及阶段性处理函数
void Upgrade(const Any&context, const ConnectedCallback &conn, 
             const MessageCallback &msg,const ClosedCallback &closed, 
             const AnyEventCallback &event);

private:
  int _conn_id;   // 唯一标识
  int _sockfd; // 套接字描述符
  EventLoop *_loop;
  // 原始代码：使用裸指针 Socket*, Channel*, Buffer*，且析构函数未 delete，导致内存泄漏
  // Socket *_socket;     // 原始代码
  // Channel *_channel;   // 原始代码
  // Buffer *_in_buffer;  // 原始代码
  // Buffer *_out_buffer; // 原始代码
  // 优化后：使用 std::unique_ptr 替代裸指针，自动管理生命周期，配合 MemoryPool operator delete 高效释放
  std::unique_ptr<Socket> _socket;     // 套接字操作管理
  std::unique_ptr<Channel> _channel;    // 链接事件管理
  std::unique_ptr<Buffer> _in_buffer;  // 输入缓冲区
  std::unique_ptr<Buffer> _out_buffer; // 输出缓冲区
  Any _context;       // 协议上下文
  bool _enable_inactive_release; // 连接是否启动非活跃销毁的判断标志,默认为false
  ConnStatu _statu;//

  using ConnectedCallback = std::function<void(const PtrConnection &)>;
  using MessageCallback = std::function<void(const PtrConnection &, Buffer *)>;
  using ClosedCallback = std::function<void(const PtrConnection &)>;
  using AnyEventCallback = std::function<void(const PtrConnection &)>;
  ConnectedCallback _connected_callback;
  MessageCallback _message_callback;
  //用户设置关闭回调
  ClosedCallback _closed_callback;
  AnyEventCallback _event_callback;
  //服务器内关闭回调
  ClosedCallback _server_closed_callback;

};
