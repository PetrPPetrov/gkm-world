# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(balancer_monitor)

cmake_policy(SET CMP0020 NEW)

set(CMAKE_PREFIX_PATH ${VCPKG_ROOT}/installed/x64-windows-static)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/../../include
${Qt5Widgets_INCLUDE_DIRS}
${Qt5Network_INCLUDE_DIRS}
)

add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)
include_directories(${VCPKG_ROOT}/installed/x64-windows-static/include)
link_directories(${VCPKG_ROOT}/installed/x64-windows-static)

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE COMMON_SOURCE ${PROJECT_SOURCE_DIR}/../../include/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

set(BALANCE_MONITOR_UI_FILES
${PROJECT_SOURCE_DIR}/main_monitor_window.ui
)

set(BALANCE_MONITOR_MOC_FILES
${PROJECT_SOURCE_DIR}/main_monitor_window.h
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME} WIN32
${BALANCE_MONITOR_UI_FILES}
${BALANCE_MONITOR_MOC_FILES}
${HEADERS}
${SOURCES}
${COMMON}
${COMMON_SOURCE}
)

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Network)
target_link_libraries(${PROJECT_NAME} optimized lib/boost_filesystem-vc140-mt debug debug/lib/boost_filesystem-vc140-mt-gd)
source_group(Common FILES ${COMMON} ${COMMON_SOURCE})
source_group(UiMoc FILES ${BALANCE_MONITOR_UI_FILES} ${BALANCE_MONITOR_MOC_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
