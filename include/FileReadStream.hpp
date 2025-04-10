#pragma once

#include <cassert>
#include <cstdio>
#include <vector>

#include "noncopyable.hpp"

namespace goa {

namespace json {

// 缓存文件内容 提供访问接口
class FileReadStream : noncopyable {
 public:
  // using 创建类型别名
  using ConstIterator = std::vector<char>::const_iterator;

  explicit FileReadStream(FILE *input) {
    char buf[65536];
    while (true) {
      size_t n = fread(buf, 1, sizeof(buf), input);
      if (n == 0) break;
      buffer_.insert(buffer_.end(), buf, buf + n);
    }

    iter_ = buffer_.cbegin();
  }

  //各类接口
  bool hasNext() const { return iter_ != buffer_.cend(); }
  char peek() const { return hasNext() ? *iter_ : '\0'; }
  ConstIterator getConstIter() const { return iter_; }
  char next() { return hasNext() ? *iter_++ : '\0'; }
  void assertNext(char c) {
    assert(peek() == c);
    next();
  }

 private:
  std::vector<char> buffer_;
  ConstIterator iter_;
};
}  // namespace json

}  // namespace goa