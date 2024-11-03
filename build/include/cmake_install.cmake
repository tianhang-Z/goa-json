# Install script for directory: /mnt/d/cs_git_rep/project/goahead-json/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/mnt/d/cs_git_rep/project/goahead-json/build/lib/libgoa-json.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/mnt/d/cs_git_rep/project/goahead-json/include/noncopyable.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/FileReadStream.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/FileWriteStream.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/StringReadStream.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/StringWriteStream.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/Value.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/Exception.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/Writer.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/Reader.hpp"
    "/mnt/d/cs_git_rep/project/goahead-json/include/Document.hpp"
    )
endif()

