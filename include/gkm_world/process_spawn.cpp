// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#ifdef _WIN32
#define _SCL_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32
#define BOOST_USE_WINDOWS_H
#define WIN32_LEAN_AND_MEAN
#include <boost/process/windows.hpp>
#endif
#include <boost/process.hpp>
#include "process_spawn.h"

std::string findNodeServerPath()
{
    return boost::process::search_path("node_server").generic_string();
}

void processSpawn(const std::string& executable_name, const std::string& node_server_port_number)
{
    boost::process::spawn(executable_name, node_server_port_number);
}
