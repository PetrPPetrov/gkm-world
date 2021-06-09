# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(log_server)

find_package(Boost 1.60 REQUIRED)

include_directories(
${PROJECT_SOURCE_DIR}/../../include
${PROJECT_SOURCE_DIR}/../../3rdparty/eigen/include/eigen3
)

add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE COMMON_SOURCE ${PROJECT_SOURCE_DIR}/../../include/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(log_server ${HEADERS} ${SOURCES} ${COMMON} ${COMMON_SOURCE})
source_group(Common FILES ${COMMON} ${COMMON_SOURCE})

if(WIN32)
  set_target_properties(log_server PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
endif()
