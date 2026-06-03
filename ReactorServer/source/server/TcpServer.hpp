#pragma once
#include "Acceptor.hpp"
#include "Connection.hpp"
#include "EventLoop.hpp"
#include "LoopThreadPool.hpp"
#include <cstdint>
#include <unordered_map>

class TcpServer{
    public:
        TcpServer(int port);
        ~TcpServer();
        void SetMessageCallback(MessageCallback cb);
        void SetConnectedCallback(ConnectedCallback cb);
        void SetClosedCallback(ClosedCallback cb);
        void SetEventCallback(AnyEventCallback cb);
        void SetEnableInactiveRelease(bool enable);
        void SetThreadNum(int thread_num);//设置子线程个数
        void SetTimeout(int timeout);//设置连接超时时间
        void RunAfter(const Functor &task, int delay);//设置定时任务
        void Start();//启动服务器
        void Stop();//终止服务器
    private:
        void NewConnection(int conn_id);
        void RemoveConnection(const PtrConnection &conn);
        void RemoveConnectionInLoop(const PtrConnection& conn);
        void RunAfterInLoop(const Functor &task, int delay);
    private:
        int _port;// 监听端口
        uint64_t _conn_id;// 连接id
        int _thread_num;// 线程数量
        int _timeout=10;// 连接超时时间
        EventLoop _base_loop;// 主线程EventLoop
        Acceptor *_acceptor;// 监控
        LoopThreadPool _pool;// 线程池
        std::unordered_map<uint64_t, PtrConnection> _connections;// 连接列表
        bool _enable_inactive_release; // 连接是否启动非活跃销毁的判断标志
        ConnectedCallback _connected_callback;
        MessageCallback _message_callback;
        //用户设置关闭回调
        ClosedCallback _closed_callback;
        AnyEventCallback _event_callback;
        //服务器内关闭回调
        ClosedCallback _server_closed_callback;


};