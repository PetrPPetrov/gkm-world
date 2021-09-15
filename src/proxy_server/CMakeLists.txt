# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(proxy_server)

include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/../../include
)

add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)

if(WIN32)
  include_directories(${VCPKG_ROOT}/installed/x64-windows-static/include)
  link_directories(${VCPKG_ROOT}/installed/x64-windows-static)
endif()

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE COMMON_SOURCE ${PROJECT_SOURCE_DIR}/../../include/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${COMMON} ${COMMON_SOURCE})
source_group(Common FILES ${COMMON} ${COMMON_SOURCE})

if(WIN32)
  target_link_libraries(${PROJECT_NAME} optimized lib/boost_filesystem-vc140-mt debug debug/lib/boost_filesystem-vc140-mt-gd)
  set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
endif()
