// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <string>
#include <iostream>
#include "boost/system/system_error.hpp"
#include "res_packer.h"

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Resource Packer Copyright (c) 2018 Petr Petrovich Petrov" << std::endl;
    std::cout << "usage: res_packer [configuration_file_name]" << std::endl;

    std::string cfg_file_name = "res_packer.cfg";
    if (argc >= 2)
    {
        cfg_file_name = argv[1];
    }

    std::cout << "using configuration file: " << cfg_file_name << std::endl;

    bool result = false;
    try
    {
        std::unique_ptr<ResPacker> res_packer = std::make_unique<ResPacker>(cfg_file_name);
        result = res_packer->start();
    }
    catch (boost::system::system_error& error)
    {
        std::cerr << "boost::system::system_error: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "std::exception: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
