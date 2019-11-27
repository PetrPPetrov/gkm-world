// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <boost/asio/impl/src.hpp>
#include "protocol.h"
#include "log.h"
// #include "balancer_server.h"

extern std::ofstream* g_log_file = nullptr;

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Balancer Server Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved." << std::endl;

    LogFileHolder log_file_holder;
    g_log_file = new std::ofstream("balancer_server.log", std::ios::app);
    LOG_INFO << "Balancer Server is starting..." << std::endl;

    try
    {
        // BalancerServer balancer_server;
        // balancer_server.start();
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
