set(HEADERS
        noncopyable.hpp
        FileReadStream.hpp
        FileWriteStream.hpp
        StringReadStream.hpp
        StringWriteStream.hpp
        Value.hpp
        Exception.hpp
        Writer.hpp
        Reader.hpp
        Document.hpp
)

add_library(goa-json STATIC ${HEADERS}) 
set_target_properties(goa-json PROPERTIES LINKER_LANGUAGE CXX)

install(TARGETS goa-json DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include)