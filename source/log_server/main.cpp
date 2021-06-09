// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <memory>
#include <string>
#include <boost/asio/impl/src.hpp>
#include "log_server.h"

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Log Server Copyright (c) 2018 Petr Petrovich Petrov" << std::endl;
    std::cout << "usage: log_server [configuration_file_name]" << std::endl;

    std::string cfg_file_name = "log_server.cfg";
    if (argc >= 2)
    {
        cfg_file_name = argv[1];
    }

    std::cout << "using configuration file: " << cfg_file_name << std::endl;

    bool result = false;
    try
    {
        auto log_server = std::make_unique<LogServer>(cfg_file_name);
        result = log_server->start();
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
