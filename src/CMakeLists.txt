# Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
# License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

set(VCPKG_ROOT CACHE PATH VCPKG_ROOT)
if(NOT VCPKG_ROOT)
  message(FATAL_ERROR "Need to specify VCPKG_ROOT")
endif()

if(WIN32)
endif()

add_subdirectory(balancer_server)
add_subdirectory(bot_client)
add_subdirectory(log_server)
add_subdirectory(node_server)
add_subdirectory(proxy_server)
add_subdirectory(vector_pool_proof)

if(WIN32)
  add_subdirectory(balancer_monitor)
  add_subdirectory(res_packer)
  add_subdirectory(win_client)
endif()
