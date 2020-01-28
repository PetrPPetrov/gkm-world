# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(bot_client)

find_package(Boost 1.60 REQUIRED)

include_directories(
${PROJECT_SOURCE_DIR}/../../include
${PROJECT_SOURCE_DIR}/../../3rdparty/eigen/include/eigen3
)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${COMMON})
source_group(Common FILES ${COMMON})

if(WIN32)
  set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
endif()
