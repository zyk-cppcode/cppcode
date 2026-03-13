#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include <iostream>

class buffer{
    public:
        buffer();
        ~buffer();
        //获取写入地址
        uint64_t getWriteOffset();
        
        //获取读取地址
        uint64_t getReadOffset();
        //获取前沿空闲空间大小--写偏移之后的空闲空间
        //获取后沿空闲空间大小--读偏移之前的空闲空间
        //获取可读数据大小
        //将读偏移向后移动
        //将写偏移向后移动(整体空闲空间够了就移动数据，否则就扩容)
        //确保可写空间足够
        //写入数据
        //读取数据
        //清空缓冲区
    private:
        std::vector<char> _buffer;
        uint64_t _WriteOffset;
        uint64_t _ReadOffset;

};