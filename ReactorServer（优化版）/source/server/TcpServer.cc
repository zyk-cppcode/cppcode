#include "TcpServer.hpp"
#include "logger.hpp"

TcpServer::TcpServer(int port)
    : _port(port),
      _conn_id(0),
      _thread_num(0),
      _timeout(10),
      _base_loop(), 
      _acceptor(nullptr),
      _pool(&_base_loop),
       _enable_inactive_release(false)
       {
    // 原始代码：使用裸指针 new，~TcpServer() 为空导致内存泄漏
    // _acceptor = new Acceptor(&_base_loop, _port);
    // 优化后：使用 std::make_unique + MemoryPool，自动管理生命周期
    _acceptor = std::make_unique<Acceptor>(&_base_loop, _port);
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
//设置非阻塞等待
void TcpServer::SetEnableInactiveRelease(bool enable){ 
    _enable_inactive_release = enable; 
}
//设置子线程个数
void TcpServer::SetThreadNum(int thread_num){
    _thread_num = thread_num;
    _pool.SetThreadCount(_thread_num);
}
void TcpServer::SetTimeout(int timeout){
     _timeout = timeout; 
}

//启动服务器
void TcpServer::Start(){
    _pool.Creat();
    _base_loop.Loop();
    LOG(LogLevel::INFO) << "服务器已启动，监听端口: " << _port;
}
//新链接回调
void TcpServer::NewConnection(int fd){
    _conn_id++;
    // 原始代码：std::make_shared 使用全局 operator new，不触发 Connection 的类级 MemoryPool operator new
    // auto conn = std::make_shared<Connection>(_conn_id, fd, _pool.GetNextLoop());
    // 优化后：使用 new Connection 触发类级 MemoryPool operator new，然后包装为 shared_ptr
    auto conn = std::shared_ptr<Connection>(new Connection(_conn_id, fd, _pool.GetNextLoop()));
    conn->SetMessageCallback(_message_callback);
    conn->SetConnectedCallback(_connected_callback);
    conn->SetClosedCallback(_closed_callback);
    conn->SetEventCallback(_event_callback);
    conn->SetServerClosedCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));
    if(_enable_inactive_release){
        conn->EnableInactiveRelease(_timeout); // 超时释放连接
    }
    conn->Established();
    _connections[_conn_id] = conn;
}
void TcpServer::RunAfter(const Functor &task, int delay) {
            _base_loop.RunInLoopThread(std::bind(&TcpServer::RunAfterInLoop, this, task, delay));
        }
void TcpServer::RunAfterInLoop(const Functor &task, int delay) {
            _conn_id++;
            _base_loop.TimerAdd(_conn_id, delay, task);
        }
//移除链接
void TcpServer::RemoveConnection(const PtrConnection &conn){
    _base_loop.RunInLoopThread(std::bind(&TcpServer::RemoveConnectionInLoop,this,conn));
}
void TcpServer::RemoveConnectionInLoop(const PtrConnection& conn){
    int conn_id = conn->Id(); 
    auto it = _connections.find(conn_id);
    if(it==_connections.end())
    {
        LOG(LogLevel::ERROR)<<"FIND CONN FAILED!";
        return;
    } 
    else
    {
        _connections.erase(it);//从链接列表移除
    }
}