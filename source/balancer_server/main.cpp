// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <memory>
#include <boost/asio/impl/src.hpp>
#include "balancer_server.h"

int main(int argc, char** argv)
{
    std::cout << "Gkm-World Balancer Server Copyright (c) 2018 Petr Petrovich Petrov" << std::endl;

    bool result = false;
    try
    {
        std::unique_ptr<BalancerServer> balancer_server = std::make_unique<BalancerServer>();
        result = balancer_server->start();
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
