# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I../server -I./

# 目标文件
TARGET = test

# 所有源文件（自动扫描）
SRCS = \
    ../server/*.cc \
    *.cc \
    test.cpp

# 直接编译链接，不产生 .o 文件
$(TARGET):
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS)

# 清理
.PHONY: clean
clean:
	rm -f $(TARGET)