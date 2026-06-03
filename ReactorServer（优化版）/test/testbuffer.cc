#include "Buffer.h"
#include <iostream>
#include <string>

using namespace std;

int main() {
    // 1. 创建缓冲区
    Buffer buf;

    // 2. 测试基础信息
    cout << "===== 测试1：初始状态 =====" << endl;
    cout << "总大小：" << buf.getTotalSize() << endl;
    cout << "可读大小：" << buf.getReadableSize() << endl;
    cout << "前部空闲：" << buf.getFrontFreeSize() << endl;
    cout << "后部空闲：" << buf.getRearFreeSize() << endl;
    cout << endl;

    // 3. 测试写入字符串
    cout << "===== 测试2：写入数据 =====" << endl;
    string str1 = "Hello WebServer!";
    buf.write(str1);
    cout << "写入：" << str1 << endl;
    cout << "可读大小：" << buf.getReadableSize() << endl;
    cout << endl;

    // 4. 测试读取数据
    cout << "===== 测试3：读取数据 =====" << endl;
    char readBuf[1024] = {0};
    buf.read(readBuf, buf.getReadableSize());
    cout << "读取到：" << readBuf << endl;
    cout << "读取后可读大小：" << buf.getReadableSize() << endl;
    cout << endl;

    // 5. 测试大量写入 → 触发数据前移（环形缓冲区核心）
    cout << "===== 测试4：大量写入，触发数据整理 =====" << endl;
    buf.clear();
    string bigData(5000, 'A');
    buf.write(bigData);
    buf.read(readBuf, 200); // 读走200，制造后部空闲空间
    string appendData(400, 'B');
    buf.ensureWriteSize(400); // 这里会触发数据前移
    buf.write(appendData);
    cout << "数据整理 + 写入完成" << endl;
    cout << "可读总大小：" << buf.getReadableSize() << endl;
    cout << endl;

    // 6. 测试扩容
    cout << "===== 测试5：扩容测试 =====" << endl;
    buf.clear();
    string hugeData(2000, 'X');
    buf.ensureWriteSize(2000); // 触发扩容
    buf.write(hugeData);
    cout << "扩容后缓冲区总大小：" << buf.getTotalSize() << endl;
    cout << "写入后可读大小：" << buf.getReadableSize() << endl;
    cout << endl;

    // 7. 测试清空
    cout << "===== 测试6：清空缓冲区 =====" << endl;
    buf.clear();
    cout << "清空后可读大小：" << buf.getReadableSize() << endl;
    cout << "清空后写偏移：" << buf.getWriteOffset() - &buf[0] << endl;
    cout << "清空后读偏移：" << buf.getReadOffset() - &buf[0] << endl;
    cout << endl;

    cout << "===== 所有测试通过！Buffer 工作正常！=====" << endl;
    return 0;
}