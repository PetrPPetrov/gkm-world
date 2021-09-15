// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <string>
#include <boost/asio/impl/src.hpp>
#include "proxy_server.h"

int main(int argc, char** argv) {
    std::cout << "Gkm-World Proxy Server Copyright (c) 2018 Petr Petrovich Petrov" << std::endl;
    std::cout << "usage: proxy_server [configuration_file_name]" << std::endl;

    std::string cfg_file_name = "proxy_server.cfg";
    if (argc >= 2) {
        cfg_file_name = argv[1];
    }

    std::cout << "using configuration file: " << cfg_file_name << std::endl;

    bool result = false;
    try {
        std::unique_ptr<ProxyServer> proxy_server = std::make_unique<ProxyServer>(cfg_file_name);
        result = proxy_server->start();
    } catch (boost::system::system_error& error) {
        std::cerr << "boost::system::system_error: " << error.what() << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception& exception) {
        std::cerr << "std::exception: " << exception.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown error" << std::endl;
        return EXIT_FAILURE;
    }
    return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
