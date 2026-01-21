cmake_minimum_required(VERSION 3.10)
project(test)

find_package(OpenCV REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED libsodium)

add_executable(test testcode.cpp)

target_include_directories(test PRIVATE ${SODIUM_INCLUDE_DIRS})


target_link_libraries(test 
    ${OpenCV_LIBS}
    ${SODIUM_LIBRARIES}
    
)

