cmake_minimum_required(VERSION 3.10)
project(lab_3)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

add_executable(find_last find_last.cpp)
add_executable(multiply multiply.cpp)