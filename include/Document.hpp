#pragma once

#include <string_view>
#include <type_traits>

#include "FileReadStream.hpp"
#include "Reader.hpp"
#include "StringReadStream.hpp"
#include "Value.hpp"

namespace goa {

namespace json {

/*
Documen的输入是string_view
Document继承自Value   Value本身就是一个json对象 Document的实例就是json对象

Document的内部依赖于Reader::parse进行json解析 并自己实现了handler接口
解析后保存到Document对象中  最终是一个Value的树状结构 可以对json进行动态修改

可以利用value的Writer接口  调用writer这一handler 将Document对象写回文件
*/
class Document : public Value {
 public:
  ParseError parse(const std::string_view &json) {
    StringReadStream is(json);
    return parseStream(is);
  }

  template <typename ReadStream,
            typename = std::enable_if_t<
                std::is_same<ReadStream, StringReadStream>::value ||
                std::is_same<ReadStream, FileReadStream>::value>>
  ParseError parseStream(ReadStream &is) {
    return Reader::parse(is, *this);
  }

  ParseError parse(const char *json, size_t len) {
    return parse(std::string_view(json, len));
  }

 public:
  bool Null() {
    addValue(Value(ValueType::TYPE_NULL));
    return true;
  }
  bool Bool(bool b) {
    addValue(Value(b));
    return true;
  }
  bool Int32(int32_t i32) {
    addValue(Value(i32));
    return true;
  }
  bool Int64(int64_t i64) {
    addValue(Value(i64));
    return true;
  }
  bool Double(double d) {
    addValue(Value(d));
    return true;
  }
  bool String(const std::string_view &s) {
    addValue(Value(s));
    return true;
  }

  bool StartObject() {
    auto value = addValue(Value(ValueType::TYPE_OBJECT));
    stack_.emplace_back(
        value);  // 仅在遇到 { 时, 将当前对象压入栈  ;  遇到 } 时, EndObject出栈
    return true;
  }
  bool Key(std::string_view s) {
    addValue(Value(s));
    return true;
  }
  bool EndObject() {
    assert(!stack_.empty());
    assert(stack_.back().type() == ValueType::TYPE_OBJECT);
    stack_.pop_back();
    return true;
  }

  bool StartArray() {
    auto value = addValue(Value(ValueType::TYPE_ARRAY));
    stack_.emplace_back(value);
    return true;
  }
  bool EndArray() {
    assert(!stack_.empty());
    assert(stack_.back().type() == ValueType::TYPE_ARRAY);
    stack_.pop_back();
    return true;
  }

 private:
  // reader每解析一个元素 都需要添加到Document对象中
  // 对于object和array  需要维护一个栈  记录当前json对象的层级
  Value *addValue(Value &&value) {
    ValueType type = value.getType();
    (void)type;
    if (seeValue_)
      assert(!stack_.empty() && "root not singular");
    else {
      // Document继承自Value Value默认初始为TYPE_NULL
      assert(type_ == ValueType::TYPE_NULL);
      seeValue_ = true;
      type_ = value.type_;
      a_ = value.a_;  // 移动赋值  需要释放原来的内存
      value.type_ = ValueType::TYPE_NULL;
      value.a_ = nullptr;
      return this;
    }

    auto &top = stack_.back();
    if (top.type() == ValueType::TYPE_ARRAY) {
      top.value->addValue(
          std::move(value));  //利用Value的addValue接口添加到数组中
      top.valueCount++;
      return const_cast<Value *>(top.lastValue());
    } else {
      assert(top.type() == ValueType::TYPE_OBJECT);
      if (top.valueCount % 2 == 0) {
        assert((type == ValueType::TYPE_STRING) && "miss quotation mark");
        key_ = std::move(value);
        top.valueCount++;
        return &key_;  //暂时记录 key_  等待value解析
      } else {
        top.value->addMember(std::move(key_), std::move(value));
        top.valueCount++;
        return const_cast<Value *>(top.lastValue());
      }
    }
  }

 private:
  struct Level {
    explicit Level(Value *value_) : value(value_), valueCount(0) {}

    ValueType type() const { return value->getType(); }

    const Value *lastValue() {
      if (type() == ValueType::TYPE_ARRAY)
        return &value->getArray().back();
      else
        return &value->getObject().back().value;
    }

    Value *value;
    int valueCount;
  };

 private:
  std::vector<Level> stack_;
  Value key_;
  bool seeValue_ = false;
};

}  // namespace json
}  // namespace goa