cmake_minimum_required(VERSION 3.10)
project(snapshottesting)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 COMPONENTS Core Test Quick Concurrent REQUIRED)
find_package(qtasyncfuture REQUIRED)
find_package(aconcurrent REQUIRED)
find_package(qtshell REQUIRED)

set(SOURCES snapshottesting.qrc
            snapshottestingadapter.cpp
            snapshottesting.cpp
            snapshottestingrenderer.cpp
            snapshottestingrule.cpp
            snapshottestingtest.cpp
            private/snapshottesting_p.h)

add_library(snapshottesting ${SOURCES})
set_target_properties(snapshottesting PROPERTIES PUBLIC_HEADER "snapshot_init.h")
target_link_libraries(snapshottesting PRIVATE Qt5::Core Qt5::Quick Qt5::Test Qt5::Concurrent aconcurrent::aconcurrent qtshell::qtshell)
target_include_directories(snapshottesting PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:>
    PRIVATE
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})

if(ENABLE_TESTING)
    add_subdirectory(testing)
endif()

install(TARGETS snapshottesting
    EXPORT snapshottestingTargets
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})
export(TARGETS snapshottesting NAMESPACE snapshottesting:: FILE snapshottestingConfig.cmake)
set(CMAKE_EXPORT_PACKAGE_REGISTRY ON)
export(PACKAGE snapshottesting)
install(EXPORT snapshottestingTargets NAMESPACE snapshottesting:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/snapshottesting FILE snapshottestingConfig.cmake)
