
#include <iostream>

#include <FileWriteStream.hpp>
#include <Writer.hpp>


using namespace goa;

/*
Writer实现了handler 将解析之后的字符输出到流中
*/
int main()
{
    json::FileWriteStream os(stdout);
    json::Writer writer(os);

    writer.StartObject();
    writer.Key("B");
    writer.StartArray();
    writer.String("ByteDance");
    writer.String("BaiDu");
    writer.EndArray();
    writer.Key("A");
    writer.String("Alibaba");
    writer.Key("T");
    writer.String("Tencent");
    writer.EndObject();
}