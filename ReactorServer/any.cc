#include <iostream>
#include <typeinfo>
#include <string>
#include <unistd.h>

class any {
public:
  void print() { std::cout << _content << std::endl; }
  any() {};
  template <typename T> any(const T &value) { _content = new Holder<T>(value); };
  any(const any &other) {
    if (other._content) {
      _content = other._content->clone();
    } else {
      _content = nullptr;
    }
  };
  ~any() {
    if (_content) {
      delete _content;
    }
  }

  template <typename T> T *get() {
    if (_content == nullptr|| typeid(T) != _content->type() ) {
      std::cout << "类型不匹配" << std::endl;
      return nullptr;
    }
    return &((Holder<T>*)_content)->_value;
  };
  void swap(any &other) { std::swap(_content, other._content); }

  any &operator=(const any &other) {
    any(other).swap(*this);
    return *this;
  }

  template <typename T> any &operator=(const T &val) {
    any(val).swap(*this);
    return *this;
  }

private:

  class Base {
  public:
  Base() = default;
    virtual ~Base()=default;
    virtual Base *clone() const = 0;
    virtual std::type_info const &type() const = 0;
  };

  template <typename T> 
  class Holder : public Base {
  public:
    Holder(){};
    Holder(const T &value) : _value(value) {};
    Base *clone() const override { return new Holder(_value); }
    const std::type_info &type() const override { return typeid(T); }
    T _value;
  };
  Base *_content = nullptr; // 父类指针指向子类对象
};

// int main() {
//   any a(2);
//   any a1('c');
//   any a2("hello");

//   a.print();
//   std::cout<<*a.get<int>()<<std::endl;
//   std::cout<<*a1.get<char>()<<std::endl;
//   std::cout<<*a2.get<const char*>()<<std::endl;

//   return 0;
// }
class Test{
    public:
        Test() {std::cout << "构造" << std::endl;}
        Test(const Test &t) {std::cout << "拷贝" << std::endl;}
        ~Test() {std::cout << "析构" << std::endl;}
};
int main()
{

    // std::any a;
    // a = 10;
    // int *pi = std::any_cast<int>(&a);
    // std::cout << *pi << std::endl;

    // a = std::string("hello");
    // std::string *ps = std::any_cast<std::string>(&a);
    // std::cout << *ps << std::endl;
    
    any a;
    {
        Test t;
        a = t;
    }
    
    // a = 10;
    // int *pa = a.get<int>();
    // std::cout << *pa << std::endl;
    // a = std::string("nihao");
    // std::string *ps = a.get<std::string>();
    // std::cout << *ps << std::endl;
    
    while(1) sleep(1);
    return 0;
}