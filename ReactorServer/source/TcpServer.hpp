#pragma once
#include "Acceptor.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "LoopThreadPool.hpp"

class TcpServer{
    public:
        TcpServer(int port, int thread_num);
        ~TcpServer();
        void SetMessageCallback(MessageCallback cb);
        void SetConnectedCallback(ConnectedCallback cb);
        void SetClosedCallback(ClosedCallback cb);
        void SetEventCallback(AnyEventCallback cb);
        void SetEnableInactiveRelease(bool enable);
        void SetThreadNum(int thread_num);
        void Start();
    private:
        void NewConnection(int sockfd);
        void RemoveConnection(const PtrConnection &conn);
        void RemoveConnectionInLoop(const PtrConnection& conn);
    private:
        int _port;// 监听端口
        int _conn_id;// 连接id
        int _thread_num;// 线程数量
        EventLoop _base_loop;// 主线程EventLoop
        Acceptor *_acceptor;// 监控
        LoopThreadPool *_pool;// 线程池
        std::unordered_map<int, PtrConnection> _connections;// 连接列表
        bool _enable_inactive_release; // 连接是否启动非活跃销毁的判断标志
        ConnectedCallback _connected_callback;
        MessageCallback _message_callback;
        //用户设置关闭回调
        ClosedCallback _closed_callback;
        AnyEventCallback _event_callback;
        //服务器内关闭回调
        ClosedCallback _server_closed_callback;


};