#pragma once

#include <string>
#include <vector>

#include "noncopyable.hpp"

namespace goa {

namespace json {

class StringWriteStream : noncopyable {
public:
  void put(char c) { buffer_.push_back(c); }
  void put(const std::string_view &str) {
    buffer_.insert(buffer_.end(), str.begin(), str.end());
  }

  std::string_view getStringView() const {
    //这里要对迭代器取值 获取第一个字符 然后对该字符取址
    return std::string_view(&*buffer_.begin(), buffer_.size());
    // return std::string_view(buffer_.data(), buffer_.size());
  }

  std::string getString() const {
    return std::string(buffer_.begin(), buffer_.end());
    // return std::string(buffer_.data(), buffer_.size());
  }

private:
  std::vector<char> buffer_;
};

} // namespace json
} // namespace goa
