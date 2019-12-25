// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <boost/asio/impl/src.hpp>
#include "log.h"
#include "node_server.h"
#include "logic_thread.h"

extern std::ofstream* g_log_file = nullptr;

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Node Server Copyright (c) 2018 by GkmSoft" << std::endl;

    std::uint16_t port_number = 17014;
    if (argc >= 2)
    {
        port_number = static_cast<std::uint16_t>(std::stoi(argv[1]));
    }
    else
    {
        std::cout << "using default port number 17014, usage is node_server port_number" << std::endl;
    }

    LogFileHolder log_file_holder;
    g_log_file = new std::ofstream("node_server" + std::to_string(port_number) + ".log", std::ios::app);
    LOG_INFO << "Node Server is starting..." << std::endl;

    try
    {
        LogicThread logic_thread;
        logic_thread.start();
        NodeServer node_server(port_number, logic_thread);
        node_server.start();
    }
    catch (boost::system::system_error& error)
    {
        LOG_FATAL << "boost::system::system_error: " << error.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& exception)
    {
        LOG_FATAL << "std::exception: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        LOG_FATAL << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
