cmake_minimum_required(VERSION 2.6)

project(ecstasy)
option(ECSTASY_BUILD_TESTS "Build the unit tests" ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

#project files to compile
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)
add_library(ecstasy STATIC ${SOURCE_FILES})
set_property(TARGET ecstasy PROPERTY CXX_STANDARD 14)
set_property(TARGET ecstasy PROPERTY CXX_STANDARD_REQUIRED ON)

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

    set_property(TARGET tests PROPERTY CXX_STANDARD 14)
    set_property(TARGET tests PROPERTY CXX_STANDARD_REQUIRED ON)
endif()
