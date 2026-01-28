cmake_minimum_required(VERSION 3.10)
project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_check_modules(SODIUM REQUIRED libsodium)
pkg_check_modules(CURLPP REQUIRED curlpp)

find_library(ZYMKEY_LIB zk REQUIRED)
find_library(ZK_CPP_LIB zkAppUtilsClassCPP REQUIRED)
set(HAILORT_LIB "/usr/lib/libhailort.so")
set(HAILORT_INCLUDE "/usr/include/hailort")
add_executable(test test_inferenceThread.cpp)

target_include_directories(test PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ${SODIUM_INCLUDE_DIRS}
    ${CURLPP_INCLUDE_DIRS}
    /usr/include/zymkey
)


target_link_libraries(test 
    ${OpenCV_LIBS}
    ${SODIUM_LIBRARIES}
    ${CURLPP_LIBRARIES}
    ${ZK_LIB}
    ${ZK_CPP_LIB}
    ${HAILORT_LIB}
)


