# 项目根目录绝对路径
ROOT_DIR = $(shell cd ../.. && pwd)
# 源文件列表
SRCS = test1.cpp \
	$(ROOT_DIR)/server/Socket.cc
# 目标文件
TARGET = test

# 编译规则
$(TARGET): $(SRCS)
	g++ $(SRCS) -o $(TARGET)
# 清理
clean:
	g++ $(SRCS) -o $(TARGET)