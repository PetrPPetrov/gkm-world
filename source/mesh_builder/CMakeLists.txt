# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(mesh_builder)

find_package(Boost 1.60 REQUIRED)

cmake_policy(SET CMP0020 NEW)

set(QT_ROOT CACHE PATH QT_ROOT)
if(NOT QT_ROOT)
  message(FATAL_ERROR "Need to specify QT_ROOT")
endif()
set(CMAKE_PREFIX_PATH ${QT_ROOT})
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

include_directories(
${PROJECT_SOURCE_DIR}/../../include
${PROJECT_SOURCE_DIR}/../../3rdparty/eigen/include/eigen3
${Qt5Widgets_INCLUDE_DIRS}
)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE COMMON_SOURCE ${PROJECT_SOURCE_DIR}/../../include/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

set(MESH_BUILDER_UI_FILES
${PROJECT_SOURCE_DIR}/main_window.ui
)

set(MESH_BUILDER_MOC_FILES
${PROJECT_SOURCE_DIR}/main_window.h
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(mesh_builder WIN32
${MESH_BUILDER_UI_FILES}
${MESH_BUILDER_MOC_FILES}
${HEADERS}
${SOURCES}
${COMMON}
${COMMON_SOURCE}
)

target_link_libraries(mesh_builder Qt5::Core Qt5::Widgets Qt5::Network)
source_group(Common FILES ${COMMON} ${COMMON_SOURCE})
source_group(UiMoc FILES ${MESH_BUILDER_UI_FILES} ${MESH_BUILDER_MOC_FILES})

if(WIN32)
  set_target_properties(mesh_builder PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
endif()
