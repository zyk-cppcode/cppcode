#pragma once

#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
class Buffer {
public:
  Buffer() : _buffer(1024), _readOffset(0), _writeOffset(0) {};
  ~Buffer() {}
  // 获取写入地址
  char *getWriteOffset() { return &_buffer[_writeOffset]; }
  // 获取读取地址
  char *getReadOffset() { return &_buffer[_readOffset]; }
  // 获取前沿空闲空间大小--写偏移之后的空闲空间
  uint64_t getFrontFreeSize() { return _buffer.size() - _writeOffset; }
  // 获取后沿空闲空间大小--读偏移之前的空闲空间
  uint64_t getRearFreeSize() { return _readOffset; }
  // 获取可读数据大小
  uint64_t getReadableSize() { return _writeOffset - _readOffset; }

  char &operator[](int index) { return _buffer[index]; }
  // 将读偏移向后移动
  void moveReadOffset(uint64_t size) {
    assert(size <= getReadableSize());
    _readOffset += size;
  }
  // 将写偏移向后移动
  void moveWriteOffset(uint64_t size) {
    assert(size <= getFrontFreeSize());
    _writeOffset += size;
  }
  // 获取总空间
  uint64_t getTotalSize() { return _buffer.size(); }
  // 获取可写空间
  uint64_t getWritableSize() { return getTotalSize() - getReadableSize(); }
  // 确保可写空间足够(整体空闲空间够了就移动数据，否则就扩容)
  void ensureWriteSize(uint64_t size) {
    if (size <= getFrontFreeSize()) {
      return;
    } else if (size <= getFrontFreeSize() + getRearFreeSize()) {
      std::copy(_buffer.begin() + _readOffset, _buffer.begin() + _writeOffset,
                _buffer.begin());
      _writeOffset = getReadableSize();
      _readOffset = 0;

    } else {
      _buffer.resize(_buffer.size() + size -
                     (getFrontFreeSize() + getRearFreeSize()));
    }
  }
  // 写入数据
  void write(const char *data, uint64_t len) {
    ensureWriteSize(len);
    std::copy(data, data + len, getWriteOffset());
    moveWriteOffset(len);
  }
  void write(std::string str) { write(&str[0], str.size()); }
  // 读取数据
  void read(char *buf, uint64_t len) {
    if (len > getReadableSize()) {
      std::cout << "可读数据不够" << std::endl;
      return;
    }
    std::copy(getReadOffset(), getReadOffset() + len, buf);
    moveReadOffset(len);
  }
  // 清空缓冲区
  void clear() {
    _writeOffset = 0;
    _readOffset = 0;
  }

private:
  std::vector<char> _buffer;
  uint64_t _writeOffset;
  uint64_t _readOffset;
};