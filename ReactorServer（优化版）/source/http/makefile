CXX = g++

# 原始编译选项：-I../server -I./
# 优化后：增加 -I../server/memorypool 以包含 MemoryPool 头文件
# CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I../server -I./
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I../server -I../server/memorypool -I./

TARGET = test

# 原始代码：只编译 ../server/*.cc
# 优化后：增加 ../server/memorypool/*.cpp，编译 MemoryPool 的 ThreadCache、CentralCache、PageCache 实现
# SRCS = ../server/*.cc *.cc test.cpp
SRCS = \
    ../server/*.cc \
    ../server/memorypool/ThreadCache.cpp \
    ../server/memorypool/CentralCache.cpp \
    ../server/memorypool/PageCache.cpp \
    *.cc \
    test.cpp

$(TARGET):
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)