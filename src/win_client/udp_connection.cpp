// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <set>
#include "udp_connection.h"
#include "main.h"

UDPConnection::EState UDPConnection::mainState() {
    return main_state;
}

void UDPConnection::start() {
    std::thread thread(&UDPConnection::runInNewThread);
    thread.detach();
}

void UDPConnection::doLogout() {
    if (exiting) {
        return;
    }

    if (main_state != EState::LoggingOut) {
#ifdef _DEBUG
        LOG_DEBUG << "doLogout" << std::endl;
#endif

        boost::asio::deadline_timer timer(io_service, boost::posix_time::milliseconds(0));
        timer.async_wait(boost::bind(&UDPConnection::onLogout, this));
    }
}

void UDPConnection::doEvent() {
    using namespace boost::placeholders;
    if (exiting) {
        return;
    }

#ifdef _DEBUG
    LOG_DEBUG << "doEvent" << std::endl;
#endif

    boost::asio::deadline_timer timer(io_service, boost::posix_time::milliseconds(0));
    timer.async_wait(boost::bind(&UDPConnection::onEvent, this, _1));
}

UDPConnection::UDPConnection() :
    timer(io_service, GAME_TIME_INTERVAL) {
    remote_end_point = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(g_server_ip_address), g_server_port_number);
    socket.connect(remote_end_point);
}

void UDPConnection::runInNewThread() {
    UDPConnection new_connection;
    g_connection = &new_connection;
    new_connection.run();
    g_connection = nullptr;
    g_is_running = false;
}

void UDPConnection::run() {
    doLogin();
    io_service.run();
}

void UDPConnection::doLogin() {
    using namespace boost::placeholders;

    if (exiting) {
        return;
    }

#ifdef _DEBUG
    LOG_DEBUG << "doLogin" << std::endl;
#endif

    main_state = EState::LoggingIn;

    auto login_request = createPacket<Packet::Login>();
    login_request->login = "game_tester";
    login_request->password = "password123";
    login_request->full_name = "game tester";
    guaranteedSend(login_request, boost::bind(&UDPConnection::onLoginFailure, this));
    setReceiveHandler(Packet::EType::LoginAnswer, boost::bind(&UDPConnection::onLoginAnswer, this, _1));
    doReceive();
}

bool UDPConnection::onLoginAnswer(size_t received_bytes) {
    if (exiting) {
        return false;
    }

#ifdef _DEBUG
    LOG_DEBUG << "onLoginAnswer" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::LoginAnswer>();
    if (main_state == EState::LoggingIn && packet->success) {
#ifdef _DEBUG
        LOG_DEBUG << "login success" << std::endl;
#endif
        g_user_token = packet->unit_token;
        main_state = EState::LoginOk;
        return doInitializePosition();
    } else {
#ifdef _DEBUG
        LOG_DEBUG << "login is not success" << std::endl;
#endif
    }

    return true;
}

void UDPConnection::onLoginFailure() {
#ifdef _DEBUG
    LOG_DEBUG << "onLoginFailure" << std::endl;
#endif

    main_state = EState::LoginFailure;
}

bool UDPConnection::doInitializePosition() {
    using namespace boost::placeholders;

    if (exiting) {
        return false;
    }

#ifdef _DEBUG
    LOG_DEBUG << "doInitializePosition" << std::endl;
#endif

    main_state = EState::InitializingPosition;

    auto request = createPacket<Packet::InitializePosition>();
    request->unit_location.unit_token = g_user_token;
    //request->user_location = g_player_state;
    guaranteedSend(request, boost::bind(&UDPConnection::onInitializePositionFailure, this));
    setReceiveHandler(Packet::EType::InitializePositionAnswer, boost::bind(&UDPConnection::onInitializePositionAnswer, this, _1));

    return true;
}

void UDPConnection::onInitializePositionFailure() {
#ifdef _DEBUG
    LOG_DEBUG << "onInitializePositionFailure" << std::endl;
#endif

    main_state = EState::InitializePositionFailure;
}

bool UDPConnection::onInitializePositionAnswer(size_t received_bytes) {
    if (exiting) {
        return false;
    }

#ifdef _DEBUG
    LOG_DEBUG << "onInitializePositionAnswer" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::InitializePositionAnswer>();
    if (main_state == EState::InitializingPosition && packet->success) {
#ifdef _DEBUG
        LOG_DEBUG << "initialize position success" << std::endl;
#endif

        //g_player_state.x_pos = packet->corrected_location.x_pos;
        //g_player_state.y_pos = packet->corrected_location.y_pos;
        //g_player_state.direction = packet->corrected_location.direction;
        main_state = EState::InitializePositionOk;
        return doTimer();
    } else {
#ifdef _DEBUG
        LOG_DEBUG << "initialize position is not success" << std::endl;
#endif
    }

    return true;
}

bool UDPConnection::doTimer() {
    using namespace boost::placeholders;

    if (exiting) {
        return false;
    }

#ifdef _DEBUG
    LOG_DEBUG << "doTimer" << std::endl;
#endif

    main_state = EState::Gaming;

    setReceiveHandler(Packet::EType::UnitActionAnswer, boost::bind(&UDPConnection::onUserActionAnswer, this, _1));
    boost::asio::deadline_timer timer(io_service, GAME_TIME_INTERVAL);
    timer.async_wait(boost::bind(&UDPConnection::onTimer, this, _1));

    return true;
}

void UDPConnection::onEvent(const boost::system::error_code& error) {
    if (exiting) {
        return;
    }

#ifdef _DEBUG
    LOG_DEBUG << "onEvent" << std::endl;
#endif
    onEventImpl();
}

void UDPConnection::onTimer(const boost::system::error_code& error) {
    using namespace boost::placeholders;

    if (exiting) {
        timer.cancel();
        return;
    } else {
        timer.expires_at(timer.expires_at() + GAME_TIME_INTERVAL);
        timer.async_wait(boost::bind(&UDPConnection::onTimer, this, _1));
    }

#ifdef _DEBUG
    LOG_DEBUG << "onTimer" << std::endl;
#endif

    if (g_logout_request && main_state != EState::LoggingOut) {
        onLogout();
        return;
    }

    onEventImpl();

#ifdef _DEBUG
    LOG_DEBUG << "cur_packet_number = " << getCurPacketNumber() << std::endl;
    //LOG_DEBUG << "local = " << g_player_state.x_pos << " : " << g_player_state.y_pos << std::endl;
#endif

    //gameStep(g_player_state, g_keyboard_state);

#ifdef _DEBUG
    LOG_DEBUG << "~onTimer" << std::endl << std::endl;
#endif
}

void UDPConnection::onEventImpl() {
    auto user_action_packet = createPacket<Packet::UnitAction>();
    user_action_packet->unit_token = g_user_token;
    user_action_packet->keyboard_state = g_keyboard_state;
    standardSend(user_action_packet);
    //packet_number_to_location.insert(packet_number_to_sent_packet_info_t::value_type(getCurPacketNumber(), { g_player_state, std::chrono::system_clock::now() }));
}

bool UDPConnection::onUserActionAnswer(size_t received_bytes) {
    if (exiting) {
        return false;
    }

#ifdef _DEBUG
    LOG_DEBUG << "onUserActionAnswer" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::UnitActionAnswer>();
    PacketNumberToSentPacketInfo::iterator fit = packet_number_to_location.find(packet->packet_number);
    if (fit == packet_number_to_location.end()) {
        LOG_ERROR << "can not find packet_number in packet_number_to_location" << std::endl;
        return true;
    }

    auto user_location = packet->unit_location;
    if (packet->packet_number <= last_received_packet_number_answer) {
#ifdef _DEBUG
        LOG_DEBUG << "cutting stale answer" << std::endl;
#endif
        return true;
    }
    last_received_packet_number_answer = packet->packet_number;

    //UnitLocation server_location = unit_location;
    SentPacketInfo history_sent_packet_info = fit->second;
    g_ping = static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - history_sent_packet_info.sending_time).count());
    //UnitLocation difference = delta(server_location, history_sent_packet_info.player_location);
    //PlayerLocation& local = g_player_state;

#ifdef _DEBUG
    LOG_DEBUG << "packet_number = " << packet->packet_number << std::endl;
    LOG_DEBUG << "history_position = " << history_sent_packet_info.player_location.x_pos << " : " << history_sent_packet_info.player_location.y_pos << std::endl;
    LOG_DEBUG << "server_position = " << server_location.x_pos << " : " << server_location.y_pos << std::endl;
    LOG_DEBUG << "difference = " << sqrt(difference.x_pos * difference.x_pos + difference.y_pos * difference.y_pos) << std::endl;
    //LOG_DEBUG << "local = " << local.x_pos << " : " << local.y_pos << std::endl;
#endif

    //PlayerLocation new_local = add(local, difference);
    //g_player_state = PlayerState(new_local.x_pos, new_local.y_pos, g_player_state.z_pos, new_local.direction);
    PacketNumberToSentPacketInfo::iterator it = fit;
    ++it;
    while (it != packet_number_to_location.end()) {
        SentPacketInfo& sent_packet_info = it->second;
        //sent_packet_info.player_location = add(sent_packet_info.player_location, difference);
        ++it;
    }
    packet_number_to_location.erase(packet_number_to_location.begin(), fit);
    packet_number_to_location.erase(fit);

#ifdef _DEBUG
    LOG_DEBUG << "other_player_count = " << packet->other_player_count << std::endl;
#endif
    typedef std::set<IndexType> UuidSet;
    UuidSet current_other_player_uuids;
    for (std::uint16_t i = 0; i < packet->other_unit_count; ++i) {
        if (i >= Packet::MAX_UNIT_COUNT_IN_PACKET) {
            break;
        }
        size_t required_size = sizeof(Packet::UnitActionAnswer) + sizeof(UnitLocationToken) * i;
        if (required_size <= received_bytes) {
#ifdef _DEBUG
            LOG_DEBUG << "other user_token = " << packet->other_player[i].user_token << std::endl;
            LOG_DEBUG << "other user x_pos = " << packet->other_player[i].x_pos << std::endl;
            LOG_DEBUG << "other user y_pos = " << packet->other_player[i].y_pos << std::endl;
            LOG_DEBUG << "other user direction = " << packet->other_player[i].direction << std::endl;
#endif
            IndexType user_token = packet->other_unit[i].unit_token;
            current_other_player_uuids.insert(user_token);
            //uuid_to_another_user_t::iterator fit = g_uuid_to_another_user->find(user_token);
            //if (fit != g_uuid_to_another_user->end())
            //{
            //    fit->second->x_pos = packet->other_player[i].x_pos;
            //    fit->second->y_pos = packet->other_player[i].y_pos;
            //    fit->second->direction = packet->other_player[i].direction;
            //}
            //else
            //{
            //    AnotherPlayer::ptr new_another_player = std::make_shared<AnotherPlayer>(packet->other_player[i]);
            //    g_uuid_to_another_user->insert(uuid_to_another_user_t::value_type(user_token, new_another_player));
            //    pushFrontBlock(new_another_player.get(), g_other_players);
            //}
        } else {
            break;
        }
    }

    //uuid_to_another_user_t::iterator user_it = g_uuid_to_another_user->begin();
    //while (user_it != g_uuid_to_another_user->end())
    //{
    //    uuid_to_another_user_t::iterator cur_user_it = user_it;
    //    ++user_it;
    //    uuid_set_t::const_iterator fit = current_other_player_uuids.find(cur_user_it->first);
    //    if (fit == current_other_player_uuids.end())
    //    {
    //        // Delete old another player
    //        removeBlock(cur_user_it->second.get(), g_other_players);
    //        g_uuid_to_another_user->erase(cur_user_it->first);
    //    }
    //}

#ifdef _DEBUG
    LOG_DEBUG << "~onUserActionAnswer" << std::endl << std::endl;
#endif

    return true;
}

void UDPConnection::onLogout() {
    using namespace boost::placeholders;

    if (exiting) {
        return;
    }

#ifdef _DEBUG
    LOG_DEBUG << "onLogout" << std::endl;
#endif

    main_state = EState::LoggingOut;
    exiting = true;
    //g_other_players = nullptr;

    auto request = createPacket<Packet::Logout>();
    request->unit_token = g_user_token;
    guaranteedSend(request, boost::bind(&UDPConnection::onLogoutFailure, this));
    setReceiveHandler(Packet::EType::LogoutAnswer, boost::bind(&UDPConnection::onLogoutAnswer, this, _1));
}

void UDPConnection::onLogoutFailure() {
#ifdef _DEBUG
    LOG_DEBUG << "onLogoutFailure" << std::endl;
#endif

    main_state = EState::LogoutFailure;
}

bool UDPConnection::onLogoutAnswer(size_t received_bytes) {
#ifdef _DEBUG
    LOG_DEBUG << "onLogoutAnswer" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::LogoutAnswer>();

    if (main_state == EState::LoggingOut) {
        if (packet->success) {
#ifdef _DEBUG
            LOG_DEBUG << "logout success" << std::endl;
#endif
            main_state = EState::LogoutOk;
        } else {
#ifdef _DEBUG
            LOG_DEBUG << "logout is not success" << std::endl;
#endif
            main_state = EState::LoginFailure;
        }
    }

    return false;
}

UDPConnection::EState UDPConnection::main_state = UDPConnection::EState::Started;
