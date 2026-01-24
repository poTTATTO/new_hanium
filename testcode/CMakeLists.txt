cmake_minimum_required(VERSION 3.10)
project(test)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

find_library(ZYMKEY_LIB zk REQUIRED)
find_library(ZK_CPP_LIB zkAppUtilsClassCPP REQUIRED)

add_executable(test test_ProcessingThread.cpp)

target_include_directories(test PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ${SODIUM_INCLUDE_DIRS}
    /usr/include/zymkey
)


target_link_libraries(test 
    ${OpenCV_LIBS}
    ${SODIUM_LIBRARIES}
    ${ZK_LIB}
    ${ZK_CPP_LIB}
    
)

