# test/Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I../ -I../spdlog
LDFLAGS = -pthread

# 源文件列表
SRC_COMMON = ../source/server/Buffer.cc \
             ../source/server/Channel.cc \
             ../source/server/Poller.cc \
             ../source/server/Socket.cc \
			 ../source/server/EventLoop.cc \
			 ../source/server/TimeWheel.cc \
			 ../source/server/Connection.cc \
			 ../source/server/Acceptor.cc \
			 ../source/server/LoopThread.cc \
			 ../source/server/LoopThreadPool.cc \
			 ../source/server/TcpServer.cc 


all: server client

# 编译 server
server: Tcpserver.cc $(SRC_COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 编译 client
client: client.cc ../source/server/Socket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f server client