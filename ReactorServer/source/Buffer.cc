#include "Buffer.hpp"
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>

Buffer::Buffer() 
    : _buffer(1024), 
      _writeOffset(0),
      _readOffset(0) 
{}

Buffer::~Buffer() {}
// 获取写入数据的起始地址
char* Buffer::getWriteOffset() {
    return &_buffer[_writeOffset];
}
// 获取读取数据的起始地址
char* Buffer::getReadOffset() {
    return &_buffer[_readOffset];
}
// 获取前面空闲的大小（缓冲区尾部空闲空间）
uint64_t Buffer::getFrontFreeSize() {
    return _buffer.size() - _writeOffset;
}
// 获取后面空闲的大小（缓冲区头部空闲空间）
uint64_t Buffer::getRearFreeSize() {
    return _readOffset;
}
// 获取可读数据的大小
uint64_t Buffer::getReadableSize() {
    return _writeOffset - _readOffset;
}

char& Buffer::operator[](int index) {
    return _buffer[index];
}
// 移动读偏移
void Buffer::moveReadOffset(uint64_t size) {
    assert(size <= getReadableSize());
    _readOffset += size;
}
// 移动写偏移
void Buffer::moveWriteOffset(uint64_t size) {
    assert(size <= getFrontFreeSize());
    _writeOffset += size;
}
// 获取总大小
uint64_t Buffer::getTotalSize() {
    return _buffer.size();
}
// 获取可写数据的大小
uint64_t Buffer::getWritableSize() {
    return getTotalSize() - getReadableSize();
}
// 确保有足够的空间写入数据
void Buffer::ensureWriteSize(uint64_t size) {
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
void Buffer::write(const char* data, uint64_t len) {
    ensureWriteSize(len);
    std::copy(data, data + len, getWriteOffset());
    moveWriteOffset(len);
}

void Buffer::write(std::string str) {
    write(&str[0], str.size());
}
// 读取数据
void Buffer::read(char* buf, uint64_t len) {
    if (len > getReadableSize()) {
        std::cout << "可读数据不够" << std::endl;
        return;
    }
    std::copy(getReadOffset(), getReadOffset() + len, buf);
    moveReadOffset(len);
}
void Buffer::read(std::string& buf, uint64_t len){
     if (len > getReadableSize()) {
        std::cout << "可读数据不够" << std::endl;
        return;
    }
    
    buf=std::string(getReadOffset(), len);
   // std::copy(getReadOffset(), getReadOffset() + len, buf.begin());
    //std::cout<<"msg:"<<buf<<std::endl;

    moveReadOffset(len);
}

//清空缓冲区
void Buffer::clear() {
    _writeOffset = 0;
    _readOffset = 0;
}