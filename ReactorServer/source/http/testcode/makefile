CXX = g++
CXXFLAGS = -std=c++17 -g -Wall
INCLUDES = -I../.. -I../../server -I../../http

# 所有源码（一次性编译，不生成 .o）
SRC = test6.cpp \
      ../../server/Socket.cc \
      ../../server/Buffer.cc \
      ../../http/HttpUtil.cc

# 只生成可执行文件，无中间文件
all:
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) -o test6

# 清理
clean:
	rm -f test6