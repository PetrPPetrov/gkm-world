// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include "gkm_world/configuration_reader.h"
#include "gkm_world/logger.h"
#include "log_server.h"

LogServer::LogServer(const std::string& cfg_file_name_) :
    cfg_file_name(cfg_file_name_), signals(io_service, SIGINT, SIGTERM) {
    setApplicationType(Packet::EApplicationType::LogServer);

    std::ifstream config_file(cfg_file_name);
    ConfigurationReader config_reader;
    config_reader.addParameter("log_server_port_number", port_number);
    config_reader.addParameter("log_min_severity", minimum_level);
    config_reader.addParameter("log_to_screen", log_to_screen);
    config_reader.addParameter("log_to_file", log_to_file);
    config_reader.read(config_file);
}

bool LogServer::start() {
    Logger::setLoggingToScreen(log_to_screen);
    Logger::setLoggingToFile(log_to_file, "node_server_" + std::to_string(port_number) + ".log");
    Logger::setMinimumLevel(minimum_level);
    Logger::registerThisThread();

    LOG_INFO << "Log Server is starting...";

    try {
        dumpParameters();
        startImpl();
    } catch (boost::system::system_error& error) {
        LOG_FATAL << "boost::system::system_error: " << error.what();
        return false;
    } catch (const std::exception& exception) {
        LOG_FATAL << "std::exception: " << exception.what();
        return false;
    } catch (...) {
        LOG_FATAL << "Unknown error";
        return false;
    }
    return true;
}

void LogServer::dumpParameters() {
    LOG_INFO << "configuration_file_name " << cfg_file_name;
    LOG_INFO << "balancer_server_ip " << log_server_end_point.address();
    LOG_INFO << "balancer_server_port_number " << log_server_end_point.port();

    LOG_INFO << "log_min_severity " << getText(minimum_level);
    LOG_INFO << "log_to_screen " << log_to_screen;
    LOG_INFO << "log_to_file " << log_to_file;
}

void LogServer::startImpl() {
    using namespace boost::placeholders;

    doReceive();
    signals.async_wait(onQuitSignal);
    setReceiveHandler(Packet::EType::LogMessage, boost::bind(&LogServer::onLogMessage, this, _1));
    setReceiveHandler(Packet::EType::LogGetMessage, boost::bind(&LogServer::onLogGetMessage, this, _1));
    io_service.run();
}

bool LogServer::onLogMessage(size_t received_bytes) {
    const auto packet = getReceiveBufferAs<Packet::LogMessage>();

    return true;
}

bool LogServer::onLogGetMessage(size_t received_bytes) {
    const auto packet = getReceiveBufferAs<Packet::LogGetMessage>();

    return true;
}
