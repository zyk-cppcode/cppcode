# test/Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I../ -I../spdlog
LDFLAGS = -pthread

# 源文件列表
SRC_COMMON = ../source/Buffer.cc \
             ../source/Channel.cc \
             ../source/Poller.cc \
             ../source/Socket.cc \
			 ../source/EventLoop.cc \
			 ../source/TimeWheel.cc \
			 ../source/Connection.cc \
			 ../source/Acceptor.cc \
			 ../source/LoopThread.cc \
			 ../source/LoopThreadPool.cc \
			 ../source/TcpServer.cc 


all: server client

# 编译 server
server: Tcpserver.cc $(SRC_COMMON)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# 编译 client
client: client.cc ../source/Socket.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f server client