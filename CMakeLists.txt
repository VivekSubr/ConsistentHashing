cmake_minimum_required(VERSION 3.0.0)
project(HashExample VERSION 1.0.0)
SET(CMAKE_CXX_FLAGS "-std=c++14")

# Setup testing
enable_testing()
find_package(GTest REQUIRED)
#set(Boost_DEBUG 1)
find_package(Boost 1.71 REQUIRED)
include_directories(${GTEST_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

# Add test cpp file
add_executable(HashTest hash_test.cc)

# Link test executable against gtest & gtest_main
target_link_libraries(HashTest ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY} ${Boost_LIBRARIES})
#target_include_directories(HashTest PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries( HashTest
    gtest_main
    gtest
    gmock
    pthread
)

add_test(
    NAME HashTest
    COMMAND HashTest
)

message(STATUS "_BOOST_INCLUDEDIR=${Boost_INCLUDE_DIRS}")