#pragma once

#include <cstdint>
#include <string>
#include <vector>

class Buffer {
public:
    Buffer();
    ~Buffer();

    char* getWriteOffset();
    char* getReadOffset();
    uint64_t getFrontFreeSize();
    uint64_t getRearFreeSize();
    uint64_t getReadableSize();

    char& operator[](int index);

    void moveReadOffset(uint64_t size);
    void moveWriteOffset(uint64_t size);
    uint64_t getTotalSize();
    uint64_t getWritableSize();

    void ensureWriteSize(uint64_t size);
    void write(const char* data, uint64_t len);
    void write(std::string str);
    void read(char* buf, uint64_t len);
    void clear();

private:
    std::vector<char> _buffer;
    uint64_t _writeOffset;
    uint64_t _readOffset;
};