/*
当一个头文件被多个源文件包含时，可能会导致重复定义的问题
#pragma once 的作用是确保一个头文件在同一个编译单元中只被包含一次。防止重复包含
类似于#ifdef 条件编译

*/
#pragma once

namespace goa {

namespace json {

class noncopyable {
public:
  noncopyable(const noncopyable &) = delete;
  void operator=(const noncopyable &) = delete;

protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

} // namespace json
} // namespace goa