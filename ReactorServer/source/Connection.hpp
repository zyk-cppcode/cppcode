#include <functional>
#include <memory>
class Socket;
class Buffer;
class Channel;
class Poller;
class Any;
class Connection;
//DISCONECTED --连接关闭状态;CONNECTING --连接建立成功-待处理状态
//CONNECTED --连接建立完成，各种设置已完成，可以通信的状态; DISCONNECTING --待关闭状态
typedef enum { DISCONECTED, CONNECTING, CONNECTED, DISCONNECTING}ConnStatu;
using PtrConnection = std::shared_ptr<Connection>;
class Connection{
    public:
    private:
    int _conn_id;//唯一标识
    int _socket_id;//套接字描述符
    Poller *_poller;
    Socket *_socket;//套接字操作管理
    Channel *channel;//链接事件管理
    Buffer *_in_buffer;//输入缓冲区
    Buffer *_out_buffer;//输出缓冲区 
    Any *_context;//协议上下文
    bool _enable_inactive_release;//连接是否启动非活跃销毁的判断标志,默认为false
    ConnStatu _statu;

    using ConnectedCallback = std::function<void(const PtrConnection&)>;
    using MessageCallback = std::function<void(const PtrConnection&, Buffer *)>;
    using ClosedCallback= std::function<void(const PtrConnection&)>;
    using AnyEventCallback= std::function<void(const PtrConnection&)>;
    ConnectedCallback _connected_callback;
    MessageCallback _message_callback;
    ClosedCallback _closed_callback;
    AnyEventCallback _event_callback;

};
