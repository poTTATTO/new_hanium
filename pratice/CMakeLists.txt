cmake_minimum_required(VERSION 3.10)
project(HaniumProject)

find_package(OpenCV REQUIRED)

add_executable(hanium_test main.cpp sharedResource.hpp save.cpp save.hpp capture.hpp capture.cpp)

target_link_libraries(hanium_test ${OpenCV_LIBS})

