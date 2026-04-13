#pragma once
#include <iostream>
#include <typeinfo>
#include <unistd.h>

class Any {
public:
  void print() { std::cout << _content << std::endl; }
  Any() {};
  template <typename T> Any(const T &value) { _content = new Holder<T>(value); };
  Any(const Any  &other) {
    if (other._content) {
      _content = other._content->clone();
    } else {
      _content = nullptr;
    }
  };
  ~Any() {
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
  void swap(Any &other) { std::swap(_content, other._content); }

  Any &operator=(const Any &other) {
    Any(other).swap(*this);
    return *this;
  }

  template <typename T> Any &operator=(const T &val) {
    Any(val).swap(*this);
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

