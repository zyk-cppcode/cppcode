#pragma once
#include<iostream>
#include<string.h>
class string{
public:
 
    // 静态常量（表示“未找到”）
    static const size_t npos;
    //友元
    friend std::ostream& operator<<(std::ostream& out,const string& s);
    // 一、构造、析构与拷贝控制
    string();                                  // 默认构造
    string(const char* cstr);                  // C风格字符串构造
    string(const string& other);               // 拷贝构造（深拷贝）
    //string(string&& other) noexcept;           // 移动构造（C++11+）
    string& operator=(const string& other);    // 拷贝赋值（深拷贝）
    //string& operator=(string&& other) noexcept;// 移动赋值（C++11+）
    ~string();                                 // 析构函数

    // 二、元素访问
    char& operator[](size_t idx);
    const char& operator[](size_t idx) const;
    char& at(size_t idx);
    const char& at(size_t idx) const;
    char& front();
    const char& front() const;
    char& back();
    const char& back() const;
    char* data() noexcept;
    const char* data() const noexcept;
    const char* c_str() const noexcept;

    // 三、容量管理
    size_t size() const noexcept;
    size_t length() const noexcept;
    size_t capacity() const noexcept;
    bool empty() const noexcept;
    void reserve(size_t new_cap);
    void shrink_to_fit();
    void clear() noexcept;

    // 四、内容修改
    void push_back(char ch);                   // 尾部追加单个字符
    string& append(const char* cstr);          // 追加C风格字符串
    string& append(const string& other);       // 追加另一个string
    string& append(size_t n, char ch);         // 追加n个重复字符
    string& operator+=(const char* cstr);      // 运算符重载：追加C字符串
    string& operator+=(const string& other);   // 运算符重载：追加string
    string& operator+=(char ch);               // 运算符重载：追加单个字符
    void insert(size_t pos, const char* cstr); // 插入C字符串（pos位置）
    void insert(size_t pos, const string& other); // 插入string
    void insert(size_t pos, size_t n, char ch); // 插入n个重复字符
    void erase(size_t pos = 0, size_t len = npos); // 删除字符（默认从0开始删完）
    string& replace(size_t pos, size_t len, const char* cstr); // 替换为C字符串
    string& replace(size_t pos, size_t len, const string& other); // 替换为string
    string& replace(size_t pos, size_t len, size_t n, char ch); // 替换为n个重复字符
    void swap(string& other) noexcept;         // 交换两个string（高效）

    // 五、查找与比较
    size_t find(const char* cstr, size_t pos = 0) const; // 查找C字符串（从pos开始）
    size_t find(const string& other, size_t pos = 0) const; // 查找string
    size_t find(char ch, size_t pos = 0) const; // 查找单个字符
    size_t rfind(const char* cstr, size_t pos = npos) const; // 反向查找C字符串
    size_t rfind(const string& other, size_t pos = npos) const; // 反向查找string
    size_t rfind(char ch, size_t pos = npos) const; // 反向查找单个字符
    int compare(const char* cstr) const;        // 与C字符串比较（返回-1/0/1）
    int compare(const string& other) const;     // 与另一个string比较
    bool operator==(const char* cstr) const;    // 相等判断（C字符串）
    bool operator==(const string& other) const; // 相等判断（string）
    bool operator!=(const char* cstr) const;    // 不等判断（C字符串）
    bool operator!=(const string& other) const; // 不等判断（string）
    bool operator<(const char* cstr) const;     // 小于判断（C字符串）
    bool operator<(const string& other) const;  // 小于判断（string）
    bool operator<=(const char* cstr) const;    // 小于等于（C字符串）
    bool operator<=(const string& other) const; // 小于等于（string）
    bool operator>(const char* cstr) const;     // 大于判断（C字符串）
    bool operator>(const string& other) const;  // 大于判断（string）
    bool operator>=(const char* cstr) const;    // 大于等于（C字符串）
    bool operator>=(const string& other) const; // 大于等于（string）

    // 六、子串提取
    string substr(size_t pos = 0, size_t len = npos) const; // 提取子串（从pos开始，取len个字符）

private:
    char* _data;
    size_t _size;
    size_t _capacity;
};

// 全局运算符重载（非成员函数，支持左操作数为C字符串）
string operator+(const string& lhs, const char* rhs);
string operator+(const char* lhs, const string& rhs);
string operator+(const string& lhs, const string& rhs);
string operator+(const string& lhs, char rhs);
string operator+(char lhs, const string& rhs);
inline std::ostream& operator<<(std::ostream& out,const string& s){
    for(int i=0;i<s._size;i++)
    {
        std::cout<<s._data[i];
    }
    return out;
}