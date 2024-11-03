#include <Document.hpp>
#include <Writer.hpp>
#include <FileWriteStream.hpp>

#include <iostream>

using namespace goa;

/*
Document接受一个string_view对象  
可对其元素进行类型判断 访问 修改 
*/
int main(){
    json::Document doc;
    auto err = doc.parse("{"
                         "    \"precision\": \"zip\","
                         "    \"Latitude\": 37.766800000000003,"
                         "    \"Longitude\": -122.3959,"
                         "    \"Address\": \"\","
                         "    \"City\": \"SAN FRANCISCO\","
                         "    \"State\": \"CA\","
                         "    \"Zip\": \"94107\","
                         "    \"Country\": \"US\""
                         "    }");

    if(err!=json::ParseError::PARSE_OK){
        std::cerr << json::parseErrorString(err) << std::endl;
        exit(1);
    }

    // operator[]   or findMember()
    // 使用operator[](const std::string_view&)必须确保doc树中必须包含"Country"的成员，否则将断言失败
    // 更安全的做法是使用Document.findMember(const std::string_view& key)
    json::Value& country = doc["Country"];
    std::cout << country.getStringView() << std::endl;
    json::Value::MemberIterator countryIter = doc.findMember("Country");
    if(countryIter!=doc.endMember()){
        std::cout << countryIter->value.getStringView() << std::endl;
    }

    // set Address to "Block 1, Street 2"
    json::Value& addr=doc["Address"];
    addr.setString("Block 1, Street 2");
    std::cout << addr.getStringView() << std::endl;


    // 向doc添加元素
    doc.addMember(json::Value("this_project"), json::Value("goa-json"));
    json::Value::MemberIterator it = doc.findMember("this_project");
    if (it != doc.endMember()){
        std::cout << it->key.getStringView() << " : " << it->value.getStringView() << std::endl;
    }

    // 结合writer 将doc写入文件
    FILE* fp = fopen("example_DOMStyle.json", "w+");
    json::FileWriteStream os(fp);
    json::Writer writer(os);
    doc.writeTo(writer);
    fclose(fp);

    return 0;
}