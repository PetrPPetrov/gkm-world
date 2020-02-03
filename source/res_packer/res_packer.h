// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <string>

class ResPacker
{
    const std::string cfg_file_name;
    std::string input_model_dir;
    std::string output_model_dir;
    std::string output_texture_dir;

public:
    ResPacker(const std::string& cfg_file_name);
    bool start();
};
