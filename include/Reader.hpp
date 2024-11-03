
# pragma once
#include <cmath>
#include <string>
#include <stdexcept>
#include <type_traits>

#include "Exception.hpp"
#include "FileReadStream.hpp"
#include "StringReadStream.hpp"
#include "Value.hpp"

namespace goa{

namespace json{

/*
    用于解析json对象 接受一个ReadStream和一个Handler作为参数
    实现对json各种数据类型的解析 包括对象、数组、字符串、数字、布尔值、null
    还包括double中的NaN和Infinity
    json本身是个object obeject的值和array的内容可以是各种类型 因此需根据情况递归解析
    并将解析结果传递给handler 利用handler处理结果
*/
class Reader: noncopyable{
public:
    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same<ReadStream, FileReadStream>::value ||
                                          std::is_same<ReadStream, StringReadStream>::value>>
    static ParseError parse(ReadStream &is, Handler &handler){
        try{
            parseWhiteSpace(is);
            parseValue(is, handler);
            parseWhiteSpace(is);
            if (is.hasNext())
                throw Exception(ParseError::PARSE_ROOT_NOT_SINGULAR);
            return ParseError::PARSE_OK;
        }
        catch (Exception &e)
        {
            return e.err();
        }
    }

private:
// throw 语句用于抛出一个异常对象。通常是继承自 std::exception 的对象，或者自定义异常类型。
#define CALL(expr) if(!(expr)) throw Exception(ParseError::PARSE_USER_STOPPED)

    // Readstream都是字节流 对字节使用next方法逐个解析

    // 解析json的转义字符 
    // \uXXXX：Unicode 字符，其中 XXXX 是四位十六进制数，表示特定的 Unicode 字符。
    template <typename ReadStream, typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                                               std::is_same_v<ReadStream, StringReadStream>>>
    static unsigned parseHex4(ReadStream &is){
        unsigned u = 0;
        for (int i = 0; i < 4;i++){
            u <<= 4;
            switch(char ch=is.next()){
                case '0'...'9': u|=ch-'0';    break;
                case 'a'...'f': u|=ch-'a'+10; break;
                case 'A'...'F': u|=ch-'A'+10; break;
                default :    throw Exception(ParseError::PARSE_BAD_UNICODE_HEX);
                }
        }
        return u;
    }

    template <typename ReadStream, typename = std::enable_if_t < std::is_same_v<ReadStream, FileReadStream> ||
                                                                 std::is_same_v<ReadStream, StringReadStream>>>
    static void parseWhiteSpace(ReadStream &is){
        while(is.hasNext()) {
            char ch=is.peek();
            if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')    is.next();
            else break;
        }
    }

    //litearl 字面量解析 
    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                          std::is_same_v<ReadStream, StringReadStream>>>
    static void parseLiteral(ReadStream &is, Handler &handler, const char *literal,ValueType type){
        char ch=*literal;

        is.assertNext(*literal++);
        while (*literal != '\0' && *literal == is.peek()){
            literal++;
            is.next();
        }
        if (*literal == '\0') {
          switch (type) {
            case ValueType::TYPE_NULL:
                CALL(handler.Null());
                return;
            case ValueType::TYPE_BOOL:
                CALL(handler.Bool(ch == 't'));
                return;
            case ValueType::TYPE_DOUBLE:
                CALL(handler.Double(ch == 'N' ? NAN : INFINITY));
                return;
            default:
                assert(false && "bad type");
            }
        }
        throw Exception(ParseError::PARSE_BAD_VALUE);
    }

    /*
    解析数字 先解析NaN和Infinity 之后是符号 
    起始不可为0  可以是double类型 支持指数形式

    */
    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                          std::is_same_v<ReadStream, StringReadStream>>>
    static void parseNumber(ReadStream &is, Handler &handler){
        if(is.peek() == 'N'){
            parseLiteral(is, handler, "NaN", ValueType::TYPE_DOUBLE);
            return;
        }
        else if(is.peek() == 'I'){
            parseLiteral(is, handler, "Infinity", ValueType::TYPE_DOUBLE);
            return;
        }

        auto start = is.getConstIter();

        if(is.peek() =='-')  is.next();

        if(is.peek() == '0'){
            is.next();
            if(isDigit(is.peek())) throw Exception(ParseError::PARSE_BAD_VALUE);
        }
        else if(isDigit19(is.peek())){
            is.next();
            while(isDigit(is.peek())) is.next();
        }
        else throw Exception(ParseError::PARSE_BAD_VALUE);

        // number有多个type 需判断
        auto expectType = ValueType::TYPE_NULL;
        
        // 小数
        if (is.peek() == '.'){
            expectType = ValueType::TYPE_DOUBLE;
            is.next();
            if(!isDigit(is.peek())) throw Exception(ParseError::PARSE_BAD_VALUE);
            while(isDigit(is.peek())) is.next();
        }

        if (is.peek() == 'e' || is.peek() == 'E'){ 
        //解析指数
            expectType = ValueType::TYPE_DOUBLE;
            is.next();
            if (is.peek() == '+' || is.peek() == '-') is.next();
            if (!isDigit(is.peek())) throw Exception(ParseError::PARSE_BAD_VALUE);
            is.next();
            while (isDigit(is.peek())) is.next();
        }

        // int32 or int64
        if(is.peek()=='i'){
            is.next();
            if (expectType == ValueType::TYPE_DOUBLE)
                throw Exception(ParseError::PARSE_BAD_VALUE);
            switch (is.next())
            {
            case '3':
                if (is.next() != '2')
                    throw Exception(ParseError::PARSE_BAD_VALUE);
                expectType = ValueType::TYPE_INT32;
                break;
            case '6':
                if (is.next() != '4')
                    throw Exception(ParseError::PARSE_BAD_VALUE);
                expectType = ValueType::TYPE_INT64;
                break;
            default:
                throw Exception(ParseError::PARSE_BAD_VALUE);
            }
        } 

        auto end = is.getConstIter();
        if (start == end) throw Exception(ParseError::PARSE_BAD_VALUE);


        try{
            //
            // std::stod() && std::stoi() are bad ideas,
            // because new string buffer is needed
            //
            // std::stoi(const string& __str, size_t* __idx = 0, int __base = 10) {
            //     return __gnu_cxx::__stoa<long, int>(&std::strtol, "stoi", __str.c_str(),
            // 		__idx, __base);
            // }
            std::size_t idx;
            if (expectType == ValueType::TYPE_DOUBLE) {
                //类型转换函数
                double d = __gnu_cxx::__stoa(&std::strtod, "stod", &*start, &idx);
                assert(start + idx == end);
                CALL(handler.Double(d));
            }
            else{
                int64_t i64 = __gnu_cxx::__stoa(&std::strtol, "stol", &*start, &idx, 10);
                if (expectType == ValueType::TYPE_INT64)
                {
                    CALL(handler.Int64(i64));
                }
                else if (expectType == ValueType::TYPE_INT32)
                {
                    if (i64 > std::numeric_limits<int32_t>::max() ||
                        i64 < std::numeric_limits<int32_t>::min())
                    {
                        throw std::out_of_range("int32_t overflow");
                    }
                    CALL(handler.Int32(static_cast<int32_t>(i64)));
                }
                else if (i64 <= std::numeric_limits<int32_t>::max() &&
                         i64 >= std::numeric_limits<int32_t>::min())
                {
                    CALL(handler.Int32(static_cast<int32_t>(i64)));
                }
                else
                {
                    CALL(handler.Int64(i64));
                }
            }
        }
        catch(std::out_of_range& e){
            throw Exception(ParseError::PARSE_NUMBER_TOO_BIG);
        }
    }

    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                          std::is_same_v<ReadStream, StringReadStream>>>
    static void parseString(ReadStream &is, Handler &handler,bool isKey){
        is.assertNext('"');
        std::string buffer;
        while(is.hasNext()){
            char ch=is.next();
            switch (ch){
                case '"':
                    if(isKey) {CALL(handler.Key(std::move(buffer)));}
                    else {CALL(handler.String(std::move(buffer)));}
                    return;
                case '\x01'...'\x1f':
                    // 此为不可打印的字符 是控制字符
                    throw Exception(ParseError::PARSE_BAD_STRING_CHAR);
                case '\\':
                    // 转义字符特殊处理 以下是json支持的转义字符
                    switch (ch=is.next()){
                        case '"': buffer.push_back('"'); break;
                        case '\\': buffer.push_back('\\'); break;
                        case '/': buffer.push_back('/'); break;
                        case 'b': buffer.push_back('\b'); break;
                        case 'f': buffer.push_back('\f'); break;
                        case 'n': buffer.push_back('\n'); break;
                        case 'r': buffer.push_back('\r'); break;
                        case 't': buffer.push_back('\t'); break;
                        case 'u': {                          
                            //  json使用\u  表示unicode码点 
                            //  json对于utf16里超出BMP的字符 使用两个/u和高低代理项 根据高低代理项可以推算unicode码点
                            unsigned u = parseHex4(is);
                            if(u>=0xD800&&u<=0xDBFF){
                                if(is.next()!='\\')
                                    throw Exception(ParseError::PARSE_BAD_UNICODE_SURROGATE);
                                if(is.next()!='u')
                                    throw Exception(ParseError::PARSE_BAD_UNICODE_SURROGATE);
                                unsigned u2 = parseHex4(is);
                                //下面根据utf16的高低代理项 计算unicode码点
                                if (u2 >= 0xDC00 && u2 <= 0xDFFF)
                                    u = 0x10000 + (u - 0xD800) * 0x400 + (u2 - 0xDC00);
                                else
                                    throw Exception(ParseError::PARSE_BAD_UNICODE_SURROGATE);
                                }
                            encodeUtf8(buffer, u);
                            break;
                        }
                        default: throw Exception(ParseError::PARSE_BAD_STRING_ESCAPE);
                    }        
                    break;
                default:buffer.push_back(ch); 
            }
        }
        throw Exception(ParseError::PARSE_MISS_QUOTATION_MARK);
    }

    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                          std::is_same_v<ReadStream, StringReadStream>>>
    static void parseArray(ReadStream &is, Handler &handler){
        CALL(handler.StartArray());

        is.assertNext('[');
        parseWhiteSpace(is);
        if(is.peek() == ']'){
            is.next();
            CALL(handler.EndArray());
            return;
        }

        while(true){
            parseValue(is, handler);
            parseWhiteSpace(is);
            switch(is.next()){
                case ',':
                    parseWhiteSpace(is);
                    break;
                case ']':
                    CALL(handler.EndArray());
                    return;
                default:
                    throw Exception(ParseError::PARSE_MISS_COMMA_OR_SQUARE_BRACKET);
            }
        }
    }

    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                          std::is_same_v<ReadStream, StringReadStream>>>
    static void parseObject(ReadStream &is, Handler &handler){
        CALL(handler.StartObject());

        is.assertNext('{');
        parseWhiteSpace(is);
        if(is.peek() == '}'){
            is.next();
            CALL(handler.EndObject());
            return;
        }

        while(true){
            // parse key
            if(is.peek() != '"')
                throw Exception(ParseError::PARSE_MISS_KEY);
            parseString(is, handler, true);
            parseWhiteSpace(is);

            if(is.next() != ':')
                throw Exception(ParseError::PARSE_MISS_COLON);
            parseWhiteSpace(is);

            // parse value  
            // value可以是各种类型 所以这里要调用value value会判断类型
            parseValue(is, handler);
            parseWhiteSpace(is);
            switch(is.next()){
                case ',':
                    parseWhiteSpace(is);
                    break;
                case '}':
                    CALL(handler.EndObject());
                    return;
                default:
                    throw Exception(ParseError::PARSE_MISS_COMMA_OR_CURLY_BRACKET);
            }
        }
    }
        
#undef CALL

    template <typename ReadStream, typename Handler,
              typename = std::enable_if_t<std::is_same_v<ReadStream, FileReadStream> ||
                                          std::is_same_v<ReadStream, StringReadStream>>>
    static void parseValue(ReadStream &is, Handler &handler){
        if(!is.hasNext())
            throw Exception(ParseError::PARSE_EXPECT_VALUE);

        switch (is.peek()) {
            case 'n': return parseLiteral(is, handler, "null", ValueType::TYPE_NULL);
            case 't': return parseLiteral(is, handler, "true", ValueType::TYPE_BOOL);
            case 'f': return parseLiteral(is, handler, "false", ValueType::TYPE_BOOL);
            case '"': return parseString(is, handler, false);
            case '[': return parseArray(is, handler);
            case '{': return parseObject(is, handler);
            default:  return parseNumber(is, handler);
        }
    }


private:
    static bool isDigit(char ch){
        return ch >= '0' && ch <= '9';
    }
    static bool isDigit19(char ch){
        return ch >= '1' && ch <= '9';
    }
    static inline void encodeUtf8(std::string &buffer, unsigned u);
        };
} //namespace json
} //namespace goa

#pragma GCC diagnostic push
// 将当前的警告设置保存在一个堆栈中，以便后续可以恢复。
#pragma GCC diagnostic ignored "-Wconversion"
// 忽略特定的警告，具体是 -Wconversion。这个警告通常是在发生类型转换时可能引发的问题，例如从较大的类型转换为较小的类型，可能导致数据丢失。

/*
函数接收一个 Unicode 码点 u（一个 unsigned 整数），
将其编码为 UTF-8 格式，结果存储在 buffer（一个 std::string）中。

unicode是为全球所有字符进行了编码
计算机里的字符流一般是ascii和utf格式
ascii utf8 和utf16是字符的三个编码形式
UTF-8和UTF-16都是Unicode标准的编码方式，支持相同的字符集。
UTF（Unicode Transformation Format）是一种用于表示 Unicode 字符集的编码方式
Unicode标准使用32位的数值来唯一标识每个字符

下面是utf8-unicode的关系
   |  Unicode符号范围      |  UTF-8编码方式
 n |  (十六进制)           | (二进制)
---+-----------------------+------------------------------------------------------
 1 | 0000 0000 - 0000 007F |                                              0xxxxxxx
 2 | 0000 0080 - 0000 07FF |                                     110xxxxx 10xxxxxx
 3 | 0000 0800 - 0000 FFFF |                            1110xxxx 10xxxxxx 10xxxxxx
 上面是基础多语言平面（BMP）的编码范围，
 下面是辅助平面
 4 | 0001 0000 - 0010 FFFF |                   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
————————————————
其中x用于保存unicode的2进制数

utf16的编码规则
（1）对于Unicode范围 U+0000 到 U+FFFF 的字符，直接用一个16位（2字节）表示
（2）超出BMP的字符：
对于Unicode范围 U+10000 到 U+10FFFF 的字符，这些字符需要使用代理对（surrogate pairs）来表示。
这些字符由一个高代理码点和一个低代理码点组成：
高代理码点：U+D800 到 U+DBFF
低代理码点：U+DC00 到 U+DFFF

cpp的string可以存储utf8格式的字符串 eg:"hello,世界"
*/
inline void goa::json::Reader::encodeUtf8(std::string &buffer, unsigned u){

    // unicode stuff from Milo's tutorial
    // 判断u在上面哪个范围内 将unicode码点 编码为 utf8格式

    switch (u)
    {
    case 0x00 ... 0x7F:
        buffer.push_back(u & 0xFF);
        break;
    case 0x080 ... 0x7FF:
        buffer.push_back(0xC0 | ((u >> 6) & 0xFF));
        buffer.push_back(0x80 | (u & 0x3F));
        break;
    case 0x0800 ... 0xFFFF:
        buffer.push_back(0xE0 | ((u >> 12) & 0xFF));
        buffer.push_back(0x80 | ((u >> 6) & 0x3F));
        buffer.push_back(0x80 | (u & 0x3F));
        break;
    case 0x010000 ... 0x10FFFF:
        buffer.push_back(0xF0 | ((u >> 18) & 0xFF));
        buffer.push_back(0x80 | ((u >> 12) & 0x3F));
        buffer.push_back(0x80 | ((u >> 6) & 0x3F));
        buffer.push_back(0x80 | (u & 0x3F));
        break;
    default:
        assert(false && "out of range");
    }
}

#pragma GCC diagnostic pop