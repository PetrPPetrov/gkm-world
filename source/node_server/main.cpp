// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <string>
#include <boost/asio/impl/src.hpp>
#include "node_server.h"
#include "logic_thread.h"

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Node Server Copyright (c) 2018 by Petr Petrovich Petrov" << std::endl;
    std::cout << "usage: node_server [port_number [configuration_file_name]]" << std::endl;

    std::uint16_t port_number = 17014;
    if (argc >= 2)
    {
        port_number = static_cast<std::uint16_t>(std::stoi(argv[1]));
    }

    std::string cfg_file_name = "node_server.cfg";
    if (argc >= 3)
    {
        cfg_file_name = argv[2];
    }

    bool result = false;
    try
    {
        std::unique_ptr<LogicThread> logic_thread = std::make_unique<LogicThread>();
        logic_thread->start();
        std::unique_ptr<NodeServer> node_server = std::make_unique<NodeServer>(port_number, cfg_file_name , *logic_thread);
        result = node_server->start();
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
