cmake_minimum_required(VERSION 3.10)

project(Hometown VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找 Boost 库
find_package(Boost REQUIRED COMPONENTS log log_setup unit_test_framework program_options random)
find_package(MySQL REQUIRED)
find_package(OpenSSL REQUIRED)
find_library(CRYPTOPP_LIBRARIES cryptopp REQUIRED)
find_library(MYSQLCPP_CONN mysqlcppconn HINTS /usr/lib/x86_64-linux-gnu)

message(STATUS "MySQL Libraries: ${MySQL_LIBRARIES}")
# 包含头文件目录
include_directories(
    ${CMAKE_BINARY_DIR}/include
    ${Boost_INCLUDE_DIRS}
)
# 配置文件
configure_file(config.h.in config.h)

# 源文件
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# 排除 main.cpp
list(REMOVE_ITEM SRC_FILES ${CMAKE_SOURCE_DIR}/src/main.cpp)
set(MAIN_FILE "${CMAKE_SOURCE_DIR}/src/main.cpp")

# 可执行文件
add_executable(Hometown ${SRC_FILES} ${MAIN_FILE})

# 包含目录
target_include_directories(Hometown PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/include"
)

# 链接库
target_link_libraries(Hometown
    ${MYSQLCPP_CONN}
    ${CRYPTOPP_LIBRARIES}
    Boost::log_setup
    Boost::log
    Boost::program_options
    OpenSSL::SSL
    OpenSSL::Crypto
)

# # 测试设置
# enable_testing()

# file(GLOB_RECURSE TEST_FILES tests/*.cpp)

# foreach(test_src ${TEST_FILES})
#     get_filename_component(test_name ${test_src} NAME_WE)
#     add_executable(${test_name} ${test_src} ${SRC_FILES})
#     target_link_libraries(${test_name} ${MYSQLCPP_CONN} ${CRYPTOPP_LIBRARIES} Boost::log_setup Boost::log Boost::unit_test_framework OpenSSL::SSL OpenSSL::Crypto)
#     target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
#     add_test(NAME ${test_name} COMMAND ${test_name})
# endforeach()