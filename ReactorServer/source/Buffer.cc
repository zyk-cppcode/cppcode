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

char* Buffer::getWriteOffset() {
    return &_buffer[_writeOffset];
}

char* Buffer::getReadOffset() {
    return &_buffer[_readOffset];
}

uint64_t Buffer::getFrontFreeSize() {
    return _buffer.size() - _writeOffset;
}

uint64_t Buffer::getRearFreeSize() {
    return _readOffset;
}

uint64_t Buffer::getReadableSize() {
    return _writeOffset - _readOffset;
}

char& Buffer::operator[](int index) {
    return _buffer[index];
}

void Buffer::moveReadOffset(uint64_t size) {
    assert(size <= getReadableSize());
    _readOffset += size;
}

void Buffer::moveWriteOffset(uint64_t size) {
    assert(size <= getFrontFreeSize());
    _writeOffset += size;
}

uint64_t Buffer::getTotalSize() {
    return _buffer.size();
}

uint64_t Buffer::getWritableSize() {
    return getTotalSize() - getReadableSize();
}

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

void Buffer::write(const char* data, uint64_t len) {
    ensureWriteSize(len);
    std::copy(data, data + len, getWriteOffset());
    moveWriteOffset(len);
}

void Buffer::write(std::string str) {
    write(&str[0], str.size());
}

void Buffer::read(char* buf, uint64_t len) {
    if (len > getReadableSize()) {
        std::cout << "可读数据不够" << std::endl;
        return;
    }
    std::copy(getReadOffset(), getReadOffset() + len, buf);
    moveReadOffset(len);
}

void Buffer::clear() {
    _writeOffset = 0;
    _readOffset = 0;
}