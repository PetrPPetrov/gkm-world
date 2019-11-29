// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <string>

std::string findNodeServerPath();
void processSpawn(const std::string& executable_name, const std::string& node_server_port_number);
