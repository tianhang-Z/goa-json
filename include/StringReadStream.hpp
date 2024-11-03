#pragma once

#include <string>
#include <cassert>

#include "noncopyable.hpp"

namespace goa{

namespace json{

class StringReadStream : noncopyable{
public:
    using ConstIterator =std::string_view::const_iterator;
    // 这里是const std::string_view &json 引用传参
    explicit StringReadStream(const std::string_view &json):
        json_(json),iter_(json.begin()){}
    
    bool          hasNext     () const {return iter_!= json_.end();}
    char          peek        () const { return hasNext()? *iter_ : '\0'; }
    ConstIterator getConstIter() const {return iter_;}
    char          next        ()       {return hasNext()? *iter_++ : '\0';}
    void          assertNext  (char c) { assert(peek() == c); next(); }


private:
    const std::string_view json_;
    ConstIterator iter_;
};

} // namespace json

} // namespace goa