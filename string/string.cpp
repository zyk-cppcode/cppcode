#include "string.h"



// 静态常量定义
const size_t string::npos = -1;

// 一、构造、析构与拷贝控制
string::string() {}

string::string(const char* cstr) {}

string::string(const string& other) {}

string::string(string&& other) noexcept {}

string& string::operator=(const string& other) {}

string& string::operator=(string&& other) noexcept {}

string::~string() {}

// 二、元素访问
char& string::operator[](size_t idx) {}

const char& string::operator[](size_t idx) const {}

char& string::at(size_t idx) {}

const char& string::at(size_t idx) const {}

char& string::front() {}

const char& string::front() const {}

char& string::back() {}

const char& string::back() const {}

char* string::data() noexcept {}

const char* string::data() const noexcept {}

const char* string::c_str() const noexcept {}

// 三、容量管理
size_t string::size() const noexcept {}

size_t string::length() const noexcept {}

size_t string::capacity() const noexcept {}

bool string::empty() const noexcept {}

void string::reserve(size_t new_cap) {}

void string::shrink_to_fit() {}

void string::clear() noexcept {}

// 四、内容修改
void string::push_back(char ch) {}

string& string::append(const char* cstr) {}

string& string::append(const string& other) {}

string& string::append(size_t n, char ch) {}

string& string::operator+=(const char* cstr) {}

string& string::operator+=(const string& other) {}

string& string::operator+=(char ch) {}

void string::insert(size_t pos, const char* cstr) {}

void string::insert(size_t pos, const string& other) {}

void string::insert(size_t pos, size_t n, char ch) {}

void string::erase(size_t pos, size_t len) {}

string& string::replace(size_t pos, size_t len, const char* cstr) {}

string& string::replace(size_t pos, size_t len, const string& other) {}

string& string::replace(size_t pos, size_t len, size_t n, char ch) {}

void string::swap(string& other) noexcept {}

// 五、查找与比较
size_t string::find(const char* cstr, size_t pos) const {}

size_t string::find(const string& other, size_t pos) const {}

size_t string::find(char ch, size_t pos) const {}

size_t string::rfind(const char* cstr, size_t pos) const {}

size_t string::rfind(const string& other, size_t pos) const {}

size_t string::rfind(char ch, size_t pos) const {}

int string::compare(const char* cstr) const {}

int string::compare(const string& other) const {}

bool string::operator==(const char* cstr) const {}

bool string::operator==(const string& other) const {}

bool string::operator!=(const char* cstr) const {}

bool string::operator!=(const string& other) const {}

bool string::operator<(const char* cstr) const {}

bool string::operator<(const string& other) const {}

bool string::operator<=(const char* cstr) const {}

bool string::operator<=(const string& other) const {}

bool string::operator>(const char* cstr) const {}

bool string::operator>(const string& other) const {}

bool string::operator>=(const char* cstr) const {}

bool string::operator>=(const string& other) const {}

// 六、子串提取
string string::substr(size_t pos, size_t len) const {}

// 全局运算符重载定义
string operator+(const string& lhs, const char* rhs) {}

string operator+(const char* lhs, const string& rhs) {}

string operator+(const string& lhs, const string& rhs) {}

string operator+(const string& lhs, char rhs) {}

string operator+(char lhs, const string& rhs) {}