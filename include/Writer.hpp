
#pragma once 

#include <cmath>
#include <cstring>

#include "Value.hpp"

namespace goa{


namespace json{

//匿名空间 仅当前文件可见
/*
提供了itoa的快速方法 频繁转换时效率更高
*/
namespace {
    
inline unsigned countDigits(unsigned n)   {
    static const uint32_t powers_of_10[] = {
        0,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000
    };
    //
    // about magic number:
    //     http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
    // __builtin_clz is gcc builtin:
    //     https://en.wikipedia.org/wiki/Bit_Manipulation_Instruction_Sets
    //     https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
    //   
    // 下面是求解n的十进制位数的数学近似方法  更高效 因为__builtin_clz是gcc是硬件指令  效率很高
    // __builtin_clz是计算前导0的数量  n|1保证至少一位有效
    uint32_t t = (32 - __builtin_clz(n | 1)) * 1233 >> 12;
    return t - (n < powers_of_10[t]) + 1;
}

inline unsigned countDigits(uint64_t n)
{
    static const uint64_t powers_of_10[] = {
        0,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000,
        10000000000,
        100000000000,
        1000000000000,
        10000000000000,
        100000000000000,
        1000000000000000,
        10000000000000000,
        100000000000000000,
        1000000000000000000,
        10000000000000000000U};
    uint32_t t = (64 - __builtin_clzll(n | 1)) * 1233 >> 12;
    return t - (n < powers_of_10[t]) + 1;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

template <typename T>
inline unsigned itoa_(T val, char *buf)
{
    static_assert(std::is_unsigned<T>::value, "must be unsigned integer");

    static const char digits[201] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    unsigned count = countDigits(val);
    unsigned next = count - 1;

    // 查digits表 将val转换为字符串
    while (val >= 100)
    {
        unsigned i = (val % 100) * 2;
        val /= 100;
        buf[next] = digits[i + 1];
        buf[next - 1] = digits[i];
        next -= 2;
    }

    /* Handle last 1-2 digits. */
    if (val < 10)
    {
        buf[next] = '0' + val;
    }
    else
    {
        unsigned i = val * 2;
        buf[next] = digits[i + 1];
        buf[next - 1] = digits[i];
    }
    return count;
}
#pragma GCC diagnostic pop

// fast int to string conversion
inline unsigned itoa(int32_t val, char *buf)
{
    auto u = static_cast<uint32_t>(val);
    if (val < 0)
    {
        *buf++ = '-';
        u = ~u + 1;
    }
    return (val < 0) + itoa_(u, buf);
}
inline unsigned itoa(int64_t val, char *buf)
{
    auto u = static_cast<uint64_t>(val);
    if (val < 0)
    {
        *buf++ = '-';
        u = ~u + 1;
    }
    return (val < 0) + itoa_(u, buf);
}

} //anonymous namespace

class StringWriteStream;
class FileWriteStream;
/*
Writer实现了handler这一接口  
用于将解析的数据发送到输出流
可用于生成json
*/
template <typename WriteStream,
          typename = std::enable_if_t<std::is_same_v<WriteStream, StringWriteStream> ||
                                     std::is_same_v<WriteStream, FileWriteStream>>>
class Writer:noncopyable{
public:
    explicit Writer(WriteStream &os):
                os_(os),seeValue_(false)  {}
    
    bool Null(){
        prefix(ValueType::TYPE_NULL);
        os_.put("null");
        return true;
    }

    bool Bool(bool b){
        prefix(ValueType::TYPE_BOOL);
        os_.put(b ? "true" : "false");
        return true;
    }
    bool Int32(int32_t i32){
        prefix(ValueType::TYPE_INT32);
        char buf[11];
        unsigned int cnt=itoa(i32,buf);
        os_.put(std::string_view(buf, cnt));
        return true;
    }

    bool Int64(int64_t i64){
        prefix(ValueType::TYPE_INT64);
        char buf[20];
        unsigned int cnt=itoa(i64,buf);
        os_.put(std::string_view(buf, cnt));
        return true;
    }

    bool Double(double d){
        prefix(ValueType::TYPE_DOUBLE);
        char buf[32];
        // 要保证缓存区足够
        if(std::isinf(d)){
            std::strcpy(buf, "Infinity");
        }
        else if(std::isnan(d)){
            std::strcpy(buf, "NaN");
        }
        else {
            // g会输出简洁形式 定点形式或者是科学计数法 会去掉尾随的0
            int n=sprintf(buf, "%.17g", d);   
            assert(n>0 && n<32);
            // ".0" in "1.0" is important to represent double type.

            auto it = std::find_if(buf, buf + n, [](char c)
                                   { return c == '.'; });  // find '.'
            if(it==buf+n) {
                strcat(buf, ".0");
            }
        }
        os_.put(buf);
        return true;
    }

    /*
    在 JSON 中，控制字符（例如 ASCII 码小于 32 的字符）需要通过 Unicode 转义序列进行表示。
    JSON 使用 \u 后跟四位十六进制数来表示这些字符。
    */
    bool String(std::string_view s){
        prefix(ValueType::TYPE_STRING);
        os_.put('"');
        for(auto c:s){
            auto u=static_cast<unsigned char>(c);
            switch(u){
                // 转义字符特殊处理  
                // json字符串中要保留转移符
                case '\"':  os_.put("\\\""); break;
                case '\\':  os_.put("\\\\"); break;
                case '\b':  os_.put("\\b"); break;
                case '\f':  os_.put("\\f"); break;   
                case '\n':  os_.put("\\n"); break;
                case '\r':  os_.put("\\r"); break;
                case '\t':  os_.put("\\t"); break;
                default:   
                    if(u<0x20){
                        char buf[7];
                        snprintf(buf, 7, "\\u%04X", u);
                        os_.put(buf);
                    }
                    else os_.put(c);
                    break;
            }
        }

        os_.put('"');
        return true;
    }

    // 下面对array和object进行处理 利用栈保存记录层级 保证括号{} []是成对的
    bool StartObject()
    {
        prefix(ValueType::TYPE_OBJECT);
        stack_.emplace_back(false);
        os_.put('{');
        return true;
    }

    bool Key(std::string_view s)
    {
        prefix(ValueType::TYPE_STRING);
        os_.put('"');
        os_.put(s);
        os_.put('"');
        return true;
    }

    bool EndObject()
    {
        assert(!stack_.empty());
        assert(!stack_.back().inArray);
        stack_.pop_back();
        os_.put('}');
        return true;
    }

    bool StartArray()
    {
        prefix(ValueType::TYPE_ARRAY);
        stack_.emplace_back(true);
        os_.put('[');
        return true;
    }

    bool EndArray()
    {
        assert(!stack_.empty());
        assert(stack_.back().inArray);
        stack_.pop_back();
        os_.put(']');
        return true;
    }

private:
    struct Level {
        explicit Level(bool inArray_):
                inArray(inArray_),valueCount(0)  { }

        bool inArray;
        int valueCount;
    };

    // 数组：每个元素之间添加“,”
    // 对象：k和v之间添加“:”，键值对之间添加“,”
    void prefix(ValueType type){
        if(seeValue_)           // stack_为空 表示解析完毕了  此后不应再调用prefix
            assert(!stack_.empty() && "root not singular");
        else
            seeValue_ = true;
        
        if(stack_.empty())
            return;
        
        Level& top=stack_.back();
        if(top.inArray){
            if(top.valueCount>0)
                os_.put(',');
        }
        else {
            if(top.valueCount%2==1)
                os_.put(':');
            else {   
                assert(type == ValueType::TYPE_STRING && "miss quotation mark");
                if(top.valueCount>0)
                    os_.put(',');
            }
        }
        top.valueCount++;
    }
    

private:
    std::vector<Level> stack_;
    WriteStream &os_;
    bool seeValue_;
};

}//namespace json

}//namespace goa