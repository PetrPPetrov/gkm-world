// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <boost/asio/impl/src.hpp>
#include "log.h"
#include "proxy_server.h"

extern std::ofstream* g_log_file = nullptr;

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Proxy Server Copyright (c) 2018 Petr Petrovich Petrov" << std::endl;

    LogFileHolder log_file_holder;
    g_log_file = new std::ofstream("proxy_server.log", std::ios::app);
    LOG_INFO << "Proxy Server is starting..." << std::endl;

    try
    {
        ProxyServer proxy_server;
        proxy_server.start();
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
