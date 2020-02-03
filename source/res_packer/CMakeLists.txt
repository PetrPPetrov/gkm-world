# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(res_packer)

find_package(Boost 1.60 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include_directories(
${PROJECT_SOURCE_DIR}/../../include
${PROJECT_SOURCE_DIR}/../../3rdparty/eigen/include/eigen3
${VCPKG_ROOT}/installed/x64-windows/include
)

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(res_packer ${HEADERS} ${SOURCES} ${COMMON})
source_group(Common FILES ${COMMON})

target_link_libraries(res_packer optimized ${VCPKG_ROOT}/installed/x64-windows/lib/turbojpeg.lib)
target_link_libraries(res_packer debug ${VCPKG_ROOT}/installed/x64-windows/debug/lib/turbojpegd.lib)
target_link_libraries(res_packer optimized ${VCPKG_ROOT}/installed/x64-windows/lib/tinyobjloader.lib)
target_link_libraries(res_packer debug ${VCPKG_ROOT}/installed/x64-windows/debug/lib/tinyobjloader.lib)

if(WIN32)
  set_target_properties(res_packer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
endif()
