cmake_minimum_required(VERSION 2.8)

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

set(CMAKE_CXX_FLAGS "-Wall -std=c++17 -g3 -O0 -fno-rtti ${LLVM_COMPILE_FLAGS}")

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})

# remove trailing whitespaces
string(STRIP "${LLVM_LDFLAGS}" LLVM_LDFLAGS)

add_executable(clang-cfg main_cfg.cc)
target_link_libraries(clang-cfg ${CLANG_LIBS} ${LLVM_LIBS_CORE} ${LLVM_LDFLAGS})

add_executable(clang-abreu main_abreu.cc)
target_link_libraries(clang-abreu ${CLANG_LIBS} ${LLVM_LIBS_CORE} ${LLVM_LDFLAGS})

