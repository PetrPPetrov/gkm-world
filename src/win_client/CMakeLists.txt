# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

project(gkm_world)

set(BGFX_ROOT CACHE PATH BGFX_ROOT)
if(NOT BGFX_ROOT)
  message(FATAL_ERROR "Need to specify BGFX_ROOT")
endif()

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_CONSTANT_MACROS)
add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_DEPRECATE)

add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)

include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/../../include
${BGFX_ROOT}/include
${BGFX_ROOT}/3rdparty
${BGFX_ROOT}/../bimg/include
${BGFX_ROOT}/../bx/include
${BGFX_ROOT}/../bx/include/compat/msvc
${BGFX_ROOT}/examples/common
${BGFX_ROOT}/examples/common/imgui
${VCPKG_ROOT}/installed/x64-windows-static/include
${CMAKE_CURRENT_BINARY_DIR}
)
link_directories(${BGFX_ROOT}/.build/win64_vs2019/bin)
link_directories(${VCPKG_ROOT}/installed/x64-windows-static)

set(SHADER_H_FILES)
set(SHADER_SOURCES
${PROJECT_SOURCE_DIR}/shaders/varying.def.sc
)

macro(compile_shader_for_single_platform shader_file_name)
  list(APPEND SHADER_H_FILES ${CMAKE_CURRENT_BINARY_DIR}/${shader_file_name}.h)
  list(APPEND SHADER_SOURCES ${PROJECT_SOURCE_DIR}/shaders/${shader_file_name}.sc)
  set(shader_flags ${ARGN})
  add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${shader_file_name}.h
  COMMAND
    ${BGFX_ROOT}/.build/win64_vs2019/bin/shadercRelease
    -f ${PROJECT_SOURCE_DIR}/shaders/${shader_file_name}.sc
    -o ${CMAKE_CURRENT_BINARY_DIR}/${shader_file_name}.h
    --bin2c
    -i ${PROJECT_SOURCE_DIR}/shaders
    -i ${BGFX_ROOT}/src
    --varyingdef ${PROJECT_SOURCE_DIR}/shaders/varying.def.sc
    ${shader_flags}
  MAIN_DEPENDENCY
    ${PROJECT_SOURCE_DIR}/shaders/${shader_file_name}.sc
  DEPENDS
    ${PROJECT_SOURCE_DIR}/shaders/varying.def.sc
  WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}/shaders
  )
endmacro()

macro(compile_vs_shader shader_file_name)
  compile_shader_for_single_platform(${shader_file_name} --type vertex --platform windows -p vs_5_0 -O 3)
endmacro()

macro(compile_fs_shader shader_file_name)
  compile_shader_for_single_platform(${shader_file_name} --type fragment --platform windows -p ps_5_0 -O 3)
endmacro()

compile_vs_shader(vs_pos_tex)
compile_fs_shader(fs_unlit)

file(GLOB_RECURSE COMMON ${PROJECT_SOURCE_DIR}/../../include/*.h)
file(GLOB_RECURSE COMMON_SOURCE ${PROJECT_SOURCE_DIR}/../../include/*.cpp)
file(GLOB_RECURSE HEADERS ${PROJECT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/*.cpp)

add_executable(${PROJECT_NAME} WIN32
${HEADERS} ${SOURCES} ${COMMON} ${COMMON_SOURCE}
${SHADER_SOURCES}
${SHADER_H_FILES}
${BGFX_ROOT}/3rdparty/dear-imgui/imgui.cpp
${BGFX_ROOT}/3rdparty/dear-imgui/imgui_demo.cpp
${BGFX_ROOT}/3rdparty/dear-imgui/imgui_draw.cpp
${BGFX_ROOT}/3rdparty/dear-imgui/imgui_widgets.cpp
${BGFX_ROOT}/examples/common/imgui/imgui.cpp
)

source_group(Common FILES ${COMMON} ${COMMON_SOURCE})
source_group(ShaderBinaries FILES ${SHADER_H_FILES})
source_group(ShadersSources FILES ${SHADER_SOURCES})
set_source_files_properties(${SHADER_H_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

target_link_libraries(${PROJECT_NAME} optimized bgfxRelease)
target_link_libraries(${PROJECT_NAME} debug bgfxDebug)

target_link_libraries(${PROJECT_NAME} optimized bimgRelease)
target_link_libraries(${PROJECT_NAME} debug bimgDebug)

target_link_libraries(${PROJECT_NAME} optimized bimg_decodeRelease)
target_link_libraries(${PROJECT_NAME} debug bimg_decodeDebug)

target_link_libraries(${PROJECT_NAME} optimized bimg_encodeRelease)
target_link_libraries(${PROJECT_NAME} debug bimg_encodeDebug)

target_link_libraries(${PROJECT_NAME} optimized bxRelease)
target_link_libraries(${PROJECT_NAME} debug bxDebug)

target_link_libraries(${PROJECT_NAME} optimized example-commonRelease)
target_link_libraries(${PROJECT_NAME} debug example-commonDebug)

target_link_libraries(${PROJECT_NAME} optimized ${VCPKG_ROOT}/installed/x64-windows-static/lib/turbojpeg.lib)
target_link_libraries(${PROJECT_NAME} debug ${VCPKG_ROOT}/installed/x64-windows-static/debug/lib/turbojpegd.lib)

target_link_libraries(${PROJECT_NAME} optimized lib/boost_filesystem-vc140-mt debug debug/lib/boost_filesystem-vc140-mt-gd)
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${RUN_AREA_DIR})
