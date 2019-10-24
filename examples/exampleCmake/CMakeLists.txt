cmake_minimum_required(VERSION 3.10)
project(snapshottesting_example)

find_package(snapshottesting REQUIRED)
find_package(qtasyncfuture REQUIRED)
find_package(aconcurrent REQUIRED)
find_package(qtshell REQUIRED)
find_package(Qt5 COMPONENTS Core Test Quick Concurrent QuickTest REQUIRED)

set(SOURCES main.cpp CustomItem.qml tst_demo1.qml)

add_executable(snapshottesting_example ${SOURCES})
target_compile_definitions(snapshottesting_example PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(snapshottesting_example PRIVATE snapshottesting::snapshottesting aconcurrent::aconcurrent qtshell::qtshell Qt5::Test Qt5::Core Qt5::Quick Qt5::QuickTest)
