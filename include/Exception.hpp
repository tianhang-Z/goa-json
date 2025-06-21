#pragma once
#include <cassert>
#include <exception>

namespace goa {

namespace json {

#define ERROR_MAP(XX)                                              \
  XX(OK, "ok")                                                     \
  XX(ROOT_NOT_SINGULAR, "root not singular")                       \
  XX(BAD_VALUE, "bad value")                                       \
  XX(EXPECT_VALUE, "expect value")                                 \
  XX(NUMBER_TOO_BIG, "number too big")                             \
  XX(BAD_STRING_CHAR, "bad character")                             \
  XX(BAD_STRING_ESCAPE, "bad escape")                              \
  XX(BAD_UNICODE_HEX, "bad unicode hex")                           \
  XX(BAD_UNICODE_SURROGATE, "bad unicode surrogate")               \
  XX(MISS_QUOTATION_MARK, "miss quotation mark")                   \
  XX(MISS_COMMA_OR_SQUARE_BRACKET, "miss comma or square bracket") \
  XX(MISS_KEY, "miss key")                                         \
  XX(MISS_COLON, "miss colon")                                     \
  XX(MISS_COMMA_OR_CURLY_BRACKET, "miss comma or curly bracket")   \
  XX(USER_STOPPED, "user stopped parse")

// 枚举ERROR_MAP中的错误类型
// {PARSE_OK,PARSE_ROOT_NOT_SINGULAR,....}
enum class ParseError : unsigned int {
#define GEN_ERRNO(e, s) PARSE_##e,
  ERROR_MAP(GEN_ERRNO)
#undef GEN_ERRNO
};

// 根据错误类型返回字符串
// tab记录ERROR_MAP中的字符串
inline const char *parseErrorString(ParseError err) {
  static const char *tab[] = {
#define GEN_STRERR(e, n) n,
      ERROR_MAP(GEN_STRERR)
#undef GEN_STRERR
  };

  assert(unsigned(err) >= 0 && unsigned(err) < sizeof(tab) / sizeof(tab[0]));
  return tab[unsigned(err)];
}

/*
自定义异常类
允许throw Exception(ParseError)
通过catch(Exception& e)获取错误类型和错误信息
*/
class Exception : public std::exception {
 public:
  // 禁止隐式类型转换
  explicit Exception(ParseError err) : err_(err) {}
  const char *errStr() const { return parseErrorString(err_); }
  ParseError err() const { return err_; }

 private:
  ParseError err_;
};

#undef ERROR_MAP
}  // namespace json

}  // namespace goa
