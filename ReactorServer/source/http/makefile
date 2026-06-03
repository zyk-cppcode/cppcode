CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -I../server -I./

TARGET = test

SRCS = \
    ../server/*.cc \
    *.cc \
    test.cpp

$(TARGET):
	$(CXX) $(SRCS) -o $(TARGET) $(CXXFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGET)