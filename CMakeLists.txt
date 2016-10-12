cmake_minimum_required(VERSION 2.6)

project(ecstasy)
option(ECSTASY_BUILD_TESTS "Build the unit tests" ON)

#warnings
if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

#project files to compile
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
add_library(ecstasy STATIC ${SOURCE_FILES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

#tests
if(ECSTASY_BUILD_TESTS)
    if(WIN32)
        set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
    endif()
    file(GLOB_RECURSE TESTS_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests_src/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests_src/*.hpp
    )
    add_executable(tests ${TESTS_SOURCE_FILES})
    target_compile_definitions(tests PUBLIC -DUSING_ECSTASY -DUSING_SIGNAL11)
    target_link_libraries(tests ecstasy)
endif()
