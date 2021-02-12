cmake_minimum_required(VERSION 3.10)

project(Blockchain VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(bc Blockchain.cpp sha256.cpp test.cpp)
target_link_libraries(bc PUBLIC pthread boost_system)