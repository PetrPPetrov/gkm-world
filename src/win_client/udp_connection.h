// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <unordered_map>
#include <chrono>
#include <thread>
#include <cmath>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "gkm_world/gkm_world.h"
#include "gkm_world/protocol.h"
#include "gkm_world/transport.h"
#include "gkm_world/logger.h"
#include "gkm_world/fast_index.h"
#include "another_player.h"

class UDPConnection : public Transport {
public:
    enum class EState : std::uint8_t {
        Started,
        LoggingIn,
        LoginFailure,
        LoginOk,
        InitializingPosition,
        InitializePositionFailure,
        InitializePositionOk,
        Gaming,
        LoggingOut,
        LogoutFailure,
        LogoutOk
    };

    struct SentPacketInfo {
        UnitLocationToken player_location;
        std::chrono::time_point<std::chrono::system_clock> sending_time;
    };

    static EState mainState();
    static void start();
    void doLogout();
    void doEvent();
private:
    UDPConnection();
    static void runInNewThread();
    void run();
    void doLogin();
    bool onLoginAnswer(size_t received_bytes);
    void onLoginFailure();
    bool doInitializePosition();
    void onInitializePositionFailure();
    bool onInitializePositionAnswer(size_t received_bytes);
    bool doTimer();
    void onEvent(const boost::system::error_code& error);
    void onTimer(const boost::system::error_code& error);
    void onEventImpl();
    bool onUserActionAnswer(size_t received_bytes);
    void onLogout();
    void onLogoutFailure();
    bool onLogoutAnswer(size_t received_bytes);
private:
    typedef std::map<IndexType, SentPacketInfo> PacketNumberToSentPacketInfo;
    PacketNumberToSentPacketInfo packet_number_to_location;

    boost::asio::deadline_timer timer;
    IndexType last_received_packet_number_answer = 0;
    bool exiting = false;
    static EState main_state;
};
