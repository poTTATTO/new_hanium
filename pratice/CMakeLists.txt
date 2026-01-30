cmake_minimum_required(VERSION 3.10)
project(HaniumProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
find_package(nlohmann_json REQUIRED)

pkg_check_modules(SODIUM REQUIRED libsodium)
pkg_check_modules(CURLPP REQUIRED curlpp)

find_library(ZYMKEY_LIB zk REQUIRED)
find_library(ZK_CPP_LIB zkAppUtilsClassCPP REQUIRED)

set(HAILORT_LIB "/usr/lib/libhailort.so")
set(HAILORT_INCLUDE "/usr/include/hailort")

add_executable(hanium_test 
    main.cpp 
    sharedResource.hpp sharedResource.cpp 
    save.cpp save.hpp 
    capture.hpp capture.cpp 
    inference.hpp inference.cpp 
    send.cpp send.hpp
    processing.hpp processing.cpp
    
)

target_include_directories(hanium_test PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    ${SODIUM_INCLUDE_DIRS}
    ${CURLPP_INCLUDE_DIRS}
    /usr/include/zymkey
)


target_link_libraries(hanium_test
    ${OpenCV_LIBS}
    ${SODIUM_LIBRARIES}
    ${CURLPP_LIBRARIES}
    ${ZK_LIB}
    ${ZK_CPP_LIB}
    ${HAILORT_LIB}
    nlohmann_json::nlohmann_json
)

