#pragma once

#include <cstdio>
#include <string_view>
/*
string_view提供了一种对字符串的非拥有性视图，
允许你以高效的方式访问字符串数据，而无需复制或管理内存。
*/
#include "noncopyable.hpp"

namespace goa{

namespace json{

/*
使用带缓冲的写，
FileWriteStream对象析构时，调用fflush清空缓冲区，防止缓冲区中暂存的数据丢失
*/
class FileWriteStream : noncopyable{
public:
    explicit FileWriteStream(FILE* output):output_(output) {}
    ~FileWriteStream() { fflush(output_); }

    void put(char c) {fputc(c, output_); }
    // 输出string_view到output_
    void put(const std::string_view& str){
        fprintf(output_, "%.*s", static_cast<int>(str.size()), str.data());
    }

private:
    FILE* output_;
};

} //namespace json

} //namespace goa