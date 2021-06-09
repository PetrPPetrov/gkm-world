// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include "gkm_world/protocol.h"
#include "gkm_world/transport.h"
#include "gkm_world/fast_index.h"

class LogServer : public Transport
{
    const std::string cfg_file_name;
    std::uint16_t port_number = 17011;
    boost::asio::signal_set signals;
    boost::asio::ip::udp::endpoint log_server_end_point;
    boost::filesystem::path node_server_path;

    Packet::ESeverityType minimum_level = Packet::ESeverityType::DebugMessage;
    bool log_to_screen = false;
    bool log_to_file = true;

public:
    LogServer(const std::string& cfg_file_name);
    bool start();

private:
    void dumpParameters();
    void startImpl();
    bool onLogMessage(size_t received_bytes);
    bool onLogGetMessage(size_t received_bytes);
};
