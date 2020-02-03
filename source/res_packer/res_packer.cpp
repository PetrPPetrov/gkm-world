// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "res_packer.h"
#include "config.h"
#include "tiny_obj_loader.h"

ResPacker::ResPacker(const std::string& cfg_file_name_) : cfg_file_name(cfg_file_name_)
{
    std::ifstream config_file(cfg_file_name);
    ConfigurationReader config_reader;
    config_reader.addParameter("input_model_dir", input_model_dir);
    config_reader.addParameter("output_model_dir", output_model_dir);
    config_reader.addParameter("output_texture_dir", output_texture_dir);
    config_reader.read(config_file);
}

bool ResPacker::start()
{
    return true;
}
