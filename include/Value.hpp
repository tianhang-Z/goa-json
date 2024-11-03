#pragma once

#include <vector>
#include <string>
#include <cstring>
#include <atomic>
#include <cassert>
#include <algorithm>

#include "noncopyable.hpp"

namespace goa{

namespace json{

enum class ValueType{
    TYPE_NULL,
    TYPE_BOOL,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_ARRAY,
    TYPE_OBJECT
};

struct Member;
class Document;

/*
value是json的基本数据类型，可以是null、bool、int32、int64、double、string、array、object
value的类型由ValueType指定，value的值由union存储，union中存储不同类型的value
一个json对象就是一个object类型，其实现是一个vector<Member>，每个Member是一个key-value对，key是string，value是Value的各种类型
可以对json实现object嵌套，即一个json对象可以可以通过键值对 让值设为另一个json对象，而json对象又可以包含数组、字符串等其他类型的值

Value类中包含了对各种类型value的封装，包括：
1. 构造函数：可以根据不同的类型构造不同的value，包括null、bool、int32、int64、double、string、array、object
2. 类型判断函数：可以判断value的类型，包括isNull、isBool、isInt32、isInt64、isDouble、isString、isArray、isObject
3. 取值函数：可以根据value的类型，获取对应的value，包括getBool、getInt32、getDouble、getArray、getObject、getString、getInt64、getStringView
4. 运算符重载：可以对value进行赋值 还可以下标访问array   利用key访问object
5. 内存管理：value的内存管理采用AddRedCount，用引用计数管理内存，当引用计数为0时，才允许释放内存; 并利用该结构体存储string array object类型

还支持对array和object的添加

*/
class Value{
    friend Document;

public:
    using MemberIterator = std::vector<Member>::iterator;
    using constMemberIterator = std::vector<Member>::const_iterator;

public:
    explicit inline Value(ValueType type = ValueType::TYPE_NULL);   
    explicit Value(bool b)              :type_(ValueType::TYPE_BOOL) ,    b_(b)       {}
    explicit Value(int32_t i32)         :type_(ValueType::TYPE_INT32), i32_(i32)  {}
    explicit Value(int64_t i64)         :type_(ValueType::TYPE_INT64), i64_(i64)  {}
    explicit Value(double d)            :type_(ValueType::TYPE_DOUBLE), d_(d)      {}
    explicit Value(std::string_view s)  :type_(ValueType::TYPE_STRING), s_(new StringWithRefCount(s.begin(),s.end())) {}
    explicit Value(const char* s)       :type_(ValueType::TYPE_STRING), s_(new StringWithRefCount(s,s+strlen(s))) {}
    Value(const char* s, size_t len)    :Value(std::string_view(s,len)) {}


    inline Value(const Value &); //拷贝构造函数
    inline Value(Value &&);      // 移动构造函数

    inline Value &operator=(const Value &); // 拷贝赋值运算符
    inline Value& operator=(Value&&)  ;  //移动赋值运算符


    inline ~Value();

public:
    ValueType getType() const { return type_; }
    inline size_t getSize() const;

    bool isNull() const { return type_ == ValueType::TYPE_NULL; }
    bool isBool() const { return type_ == ValueType::TYPE_BOOL; }
    bool isInt32() const { return type_ == ValueType::TYPE_INT32; }
    bool isInt64() const { return type_ == ValueType::TYPE_INT64||type_ == ValueType::TYPE_INT32; }
    bool isDouble() const { return type_ == ValueType::TYPE_DOUBLE; }
    bool isString() const { return type_ == ValueType::TYPE_STRING; }
    bool isArray() const { return type_ == ValueType::TYPE_ARRAY; }
    bool isObject() const { return type_ == ValueType::TYPE_OBJECT; }


    bool        getBool  () const { assert(type_ == ValueType::TYPE_BOOL);   return b_; }
    int32_t     getInt32 () const { assert(type_ == ValueType::TYPE_INT32);  return i32_; }
    double      getDouble() const { assert(type_ == ValueType::TYPE_DOUBLE); return d_; }
    const auto& getArray () const { assert(type_ == ValueType::TYPE_ARRAY);  return a_->data; }
    const auto& getObject() const { assert(type_ == ValueType::TYPE_OBJECT); return o_->data; }
    std::string getString() const { return std::string(getStringView()); }
    
    int64_t getInt64() const
    {
        assert(type_ == ValueType::TYPE_INT64 || type_ == ValueType::TYPE_INT32);
        return type_ == ValueType::TYPE_INT64 ? i64_ : i32_;
    }
    std::string_view getStringView() const
    {
        assert(type_ == ValueType::TYPE_STRING);
        return std::string_view(&*s_->data.begin(), s_->data.size());
    }

    // placement new 用于在已有的内存上构造对象 用法： new (pointer) Type(arguments);
    Value& setNull() { this->~Value(); return *new(this) Value(ValueType::TYPE_NULL); } 
    Value& setBool(bool b) { this->~Value(); return *new(this) Value(b); }
    Value& setInt32(int32_t i32) { this->~Value(); return *new(this) Value(i32); }
    Value& setInt64(int64_t i64) { this->~Value(); return *new(this) Value(i64); }
    Value& setDouble(double d) { this->~Value(); return *new(this) Value(d); }
    Value& setArray() {this->~Value(); return *new(this) Value(ValueType::TYPE_ARRAY);}
    Value& setObject() {this->~Value(); return *new(this) Value(ValueType::TYPE_OBJECT);}
    Value& setString(std::string_view s) { this->~Value(); return *new(this) Value(s); }

    inline Value &operator[](const std::string_view &); // non-const obj invokes this.
    inline Value &operator[](const std::string_view &) const; // non-const obj invokes this.

    // json迭代器 
    MemberIterator      beginMember()        { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.begin(); }
    constMemberIterator cbeginMember() const { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.cbegin(); }
    MemberIterator      endMember()          { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.end(); }
    constMemberIterator cendMember() const   { assert(type_ == ValueType::TYPE_OBJECT); return o_->data.cend(); }

    constMemberIterator beginMember() const  { return cbeginMember(); }      // const obj invokes this.
    constMemberIterator endMember() const    { return cendMember(); }        // const obj invokes this.

    inline MemberIterator      findMember(const std::string_view& key);
    inline constMemberIterator findMember(const std::string_view& key) const; // const obj invokes this.

    // 添加member
    // sdt::forward<Args>(args) 将参数以原始的值类别（左值或右值）转发到另一个函数中
    template <typename V>
    Value &addMember(const char* k, V&& v) { return addMember(Value(k), Value(std::forward<V>(v))); }

    inline Value& addMember(Value&&, Value&&);

    //对array添加
    template<typename T>
    Value& addValue(T&& value){
        assert(type_ == ValueType::TYPE_ARRAY);
        a_->data.emplace_back(std::forward<T>(value));
        return a_->data.back();
    }

    // 对array实现下标访问
    Value&       operator[](size_t i)       { assert(type_ == ValueType::TYPE_ARRAY); return a_->data[i]; }
    const Value& operator[](size_t i) const { assert(type_ == ValueType::TYPE_ARRAY); return a_->data[i]; }

    //调用handler
    template <typename Handler>
    inline bool writeTo(Handler&) const;

private:
    //json string array object 类型的结构体模板
    template<typename T,typename = std::enable_if_t<std::is_same_v<T,std::vector<char>>||
                                                    std::is_same_v<T,std::vector<Value>>||
                                                    std::is_same_v<T,std::vector<Member>>>>
    struct AddRefCount{
        template <typename... Args>
        //可变参数模板 使用时不需要知名参数类型 编译器会自动推导
        //1.利用右值 2.使用完美转发  ...用于展开参数包 将args依次发送到容器data 
        AddRefCount(Args&&... args): refCount(1),data(std::forward<Args>(args)...){}
        ~AddRefCount() { assert(refCount == 0); } 

        int incrAndGet() {assert(refCount > 0); return ++refCount;}
        int decrAndGet() {assert(refCount > 0); return --refCount;}

        //定义原子类型变量 支持 load store fetch_add fetch_sub 等操作 
        std::atomic_int refCount;
        T data;
    };

    // using 定义类型别名 定义不同的AddRefCount结构体类型 
    using StringWithRefCount = AddRefCount<std::vector<char>>;     //json string类型 保存字符串
    using ArrayWithRefCount = AddRefCount<std::vector<Value>>;       //json array类型 保存json值
    using ObjectWithRefCount = AddRefCount<std::vector<Member>>;      // json object类型 保存键值对

    ValueType type_;

    union {
        bool b_;
        int32_t i32_;
        int64_t i64_;
        double d_;
        StringWithRefCount* s_; //结构体指针 
        ArrayWithRefCount* a_;
        ObjectWithRefCount* o_;
    };

}; // end of class Value


// 这个结构体用于保存json object类型 保存键值对 其中键一般是string 值可以是任意json类型
// 每个object视为一个成员
struct Member{
    // &&右值引用 和 std::move一起使用 这样会调用移动构造函数
    Member(Value&& k,Value&& v) : key(std::move(k)), value(std::move(v)) {}
    Member(const std::string_view& k, Value&& v) : key(k), value(std::move(v)) {}

    Value key;
    Value value;
};

// definition of class Value's member functions

// 构造函数
inline Value::Value(ValueType type):
    type_(type),
    s_(nullptr){
    switch(type_){
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:                                break;
        case ValueType::TYPE_STRING: s_ = new StringWithRefCount(); break;      //此处开辟内存空间 利用AddRefCount模板
        case ValueType::TYPE_ARRAY:  a_ = new ArrayWithRefCount();  break;
        case ValueType::TYPE_OBJECT: o_ = new ObjectWithRefCount(); break;
        default :assert(false && "bad type when Value construct");
    }
}

// 这里浅拷贝  但使用引用计数 引用大于0原内存空间就不会被析构
inline Value::Value(const Value& rhs):
    type_(rhs.type_),
    s_(rhs.s_){
    switch (type_){
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:                     break;
        case ValueType::TYPE_STRING: s_ ->incrAndGet() ; break;
        case ValueType::TYPE_ARRAY:  a_ ->incrAndGet() ; break;
        case ValueType::TYPE_OBJECT: o_ ->incrAndGet() ; break;
        default :assert(false && "bad type when Value copy-construct");
    }
}

inline Value::Value(Value&& rhs) :
    type_(rhs.type_),
    s_(rhs.s_){
    rhs.type_ = ValueType::TYPE_NULL;
    rhs.s_ = nullptr;                  //原右值失效
}

// 类似拷贝构造 
inline Value& Value::operator=(const Value& rhs) {
    if (this == &rhs)  return *this;      //copy itself

    this->~Value();
    type_ = rhs.type_;
    s_ = rhs.s_;
    switch (type_){
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:                     break;
        case ValueType::TYPE_STRING: s_ ->incrAndGet() ; break;
        case ValueType::TYPE_ARRAY:  a_ ->incrAndGet() ; break;
        case ValueType::TYPE_OBJECT: o_ ->incrAndGet() ; break;
        default :assert(false && "bad type when Value copy");
    }
    return *this;
}

// 移动赋值
inline Value& Value::operator=(Value&& rhs) {
    if(this == &rhs) return *this;

    this->~Value();
    type_ = rhs.type_;
    s_ = rhs.s_;
    rhs.type_ = ValueType::TYPE_NULL;
    rhs.s_ = nullptr;                  //原右值失效
    return *this;
}

// 析构函数
// 对于非基础数据类型 需判断引用计数是否为0 若为0 则释放内存空间
inline Value::~Value() {
    switch (type_){
        case ValueType::TYPE_NULL:
        case ValueType::TYPE_BOOL:
        case ValueType::TYPE_INT32:
        case ValueType::TYPE_INT64:
        case ValueType::TYPE_DOUBLE:             break;
        case ValueType::TYPE_STRING: 
            if(s_->decrAndGet() == 0) delete s_; 
            break;
        case ValueType::TYPE_ARRAY:  
            if(a_->decrAndGet() == 0) delete a_; 
            break;
        case ValueType::TYPE_OBJECT: 
            if(o_->decrAndGet() == 0) delete o_;
            break;
        default :assert(false && "bad type when Value deconstruct");
    }
}

//此处是数据的数量 而不是内存大小
inline size_t Value::getSize() const {
    if(type_ == ValueType::TYPE_ARRAY) return a_->data.size();
    else if(type_ == ValueType::TYPE_OBJECT) return o_->data.size();
    //对于非array 非object 数量为1
    return 1;
}

// 对object类型 用key访问
inline Value& Value::operator[](const std::string_view& key) {
    assert(type_ == ValueType::TYPE_OBJECT);

    auto iter=findMember(key);
    if(iter != o_->data.end())
        return iter->value;

    assert(false);
    static Value fake(ValueType::TYPE_NULL);
    return fake;
}


inline Value& Value::operator[](const std::string_view& key) const {
    return const_cast<Value&>(*this)[key];
}

inline Value::MemberIterator Value::findMember(const std::string_view& key) {
    assert(type_ == ValueType::TYPE_OBJECT);
    return std::find_if(o_->data.begin(), o_->data.end(), 
                     [key](const Member& m){ return m.key.getStringView() == key;   });
}

inline Value::constMemberIterator Value::findMember(const std::string_view& key) const {
    return const_cast<Value&>(*this).findMember(key);
}

inline Value& Value::addMember(Value&& k, Value&& v) {
    assert(type_ == ValueType::TYPE_OBJECT);
    assert(k.type_ == ValueType::TYPE_STRING);
    assert(findMember(k.getStringView()) == endMember());
    o_->data.emplace_back(std::move(k), std::move(v));  //std::move 对象转换为右值引用 然后调用移动构造或赋值函数
    return o_->data.back().value;

}



#define CALL(expr) do{if (!(expr)) return false;} while (false)

/*
用于写json数据
*/
template <typename Handler>
inline bool Value::writeTo(Handler& handler) const {
    switch (type_) {
        case ValueType::TYPE_NULL:
            CALL(handler.Null());  //类型检查
            break;
        case ValueType::TYPE_BOOL:
            CALL(handler.Bool(b_));
            break;
        case ValueType::TYPE_INT32:
            CALL(handler.Int32(i32_));
            break;
        case ValueType::TYPE_INT64:
            CALL(handler.Int64(i64_));
            break;
        case ValueType::TYPE_DOUBLE:
            CALL(handler.Double(d_));
            break;
        case ValueType::TYPE_STRING:
            CALL(handler.String(getStringView()));
            break;
        case ValueType::TYPE_ARRAY:
            CALL(handler.StartArray());
            for (auto &val : getArray())
                CALL(val.writeTo(handler));                 //递归写入
            CALL(handler.EndArray());
            break;
        case ValueType::TYPE_OBJECT:
            CALL(handler.StartObject());
            for (auto &member : getObject()){
                handler.Key(member.key.getStringView());
                CALL(member.value.writeTo(handler));
            }
            CALL(handler.EndObject());
            break;
        default:
            assert(false && "bad type when writeTo.");
        }
        return true;
}

#undef CALL

} // namespace json
}//namespace goa