// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <string>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include "global_types.h"
#include "logic.h"
#include "protocol_enum.h"
#include "mac_address.h"
#include "balance_tree/common.h"

typedef boost::asio::ip::address_v6 ip_address_t;
#define to_v to_v6

#pragma pack(push, 1)

namespace Packet
{
    static const std::size_t MAX_SIZE = 1024;
    static const std::uint16_t MAX_USER_COUNT_IN_PACKET = 20;

    struct Base
    {
        EType type;
        std::uint32_t packet_number = 0;
    };

    struct Login : public Base
    {
        char login[64];
        char password[64];
        char full_name[64];

        Login()
        {
            type = EType::Login;
            memset(login, 0, std::size(login));
            memset(password, 0, std::size(password));
            memset(full_name, 0, std::size(full_name));
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
        std::string getLogin() const
        {
            char temp_login[sizeof(login) / sizeof(login[0])];
            memcpy(temp_login, login, std::size(login));
            temp_login[std::size(temp_login) - 1] = 0;
            return temp_login;
        }
        void setLogin(const std::string& login_)
        {
            memset(login, 0, std::size(login));
            memcpy(login, login_.c_str(), std::min(login_.size(), std::size(login)));
        }
        std::string getPassword() const
        {
            char temp_password[sizeof(password) / sizeof(password[0])];
            memcpy(temp_password, password, std::size(password));
            temp_password[std::size(temp_password) - 1] = 0;
            return temp_password;
        }
        void setPassword(const std::string& password_)
        {
            memset(password, 0, std::size(password));
            memcpy(password, password_.c_str(), std::min(password_.size(), std::size(password)));
        }
        std::string getFullName() const
        {
            char temp_full_name[sizeof(full_name) / sizeof(full_name[0])];
            memcpy(temp_full_name, full_name, std::size(full_name));
            temp_full_name[std::size(temp_full_name) - 1] = 0;
            return temp_full_name;
        }
        void setFullName(const std::string& full_name_)
        {
            memset(full_name, 0, std::size(full_name));
            memcpy(full_name, full_name_.c_str(), std::min(full_name_.size(), std::size(full_name)));
        }
    };

    struct LoginAnswer : public Base
    {
        bool success = false;
        std::uint32_t user_token = 0;

        LoginAnswer()
        {
            type = EType::LoginAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct Logout : public Base
    {
        std::uint32_t user_token = 0;

        Logout()
        {
            type = EType::Logout;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogoutAnswer : public Base
    {
        bool success = false;

        LogoutAnswer()
        {
            type = EType::LogoutAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogoutInternal : public Base
    {
        std::uint32_t user_token = 0;
        std::uint32_t client_packet_number = 0;

        LogoutInternal()
        {
            type = EType::LogoutInternal;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogoutInternalAnswer : public Base
    {
        bool success = false;
        std::uint32_t user_token = 0;
        std::uint32_t client_packet_number = 0;

        LogoutInternalAnswer()
        {
            type = EType::LogoutInternalAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePosition : public Base
    {
        std::uint32_t user_token = 0;
        PlayerLocation user_location;

        InitializePosition()
        {
            type = EType::InitializePosition;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePositionAnswer : public Base
    {
        bool success = false;
        PlayerLocation corrected_location;

        InitializePositionAnswer()
        {
            type = EType::InitializePositionAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePositionInternal : public Base
    {
        std::uint32_t user_token = 0;
        PlayerLocation user_location;
        std::uint32_t client_packet_number = 0;
        std::uint32_t proxy_packet_number = 0;
        ip_address_t::bytes_type proxy_server_address = { 0 };
        std::uint16_t proxy_server_port_number = 0;

        InitializePositionInternal()
        {
            type = EType::InitializePositionInternal;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePositionInternalAnswer : public Base
    {
        bool success = false;
        ip_address_t::bytes_type node_server_address = { 0 };
        std::uint16_t node_server_port_number = 0;
        std::uint32_t user_token = 0;
        PlayerLocation corrected_location;
        std::uint32_t client_packet_number = 0;
        std::uint32_t proxy_packet_number = 0;

        InitializePositionInternalAnswer()
        {
            type = EType::InitializePositionInternalAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UserAction : public Base
    {
        std::uint32_t user_token = 0;
        KeyboardState keyboard_state;

        UserAction()
        {
            type = EType::UserAction;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UserActionAnswer : public Base
    {
        PlayerLocation user_location;
        std::uint16_t other_player_count = 0;
        PlayerUuidLocation other_player[1];

        UserActionAnswer()
        {
            type = EType::UserActionAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UserActionInternal : public Base
    {
        std::uint32_t user_token = 0;
        KeyboardState keyboard_state;
        std::uint32_t client_packet_number = 0;

        UserActionInternal()
        {
            type = EType::UserActionInternal;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UserActionInternalAnswer : public Base
    {
        std::uint32_t user_token = 0;
        PlayerLocation user_location;
        std::uint32_t client_packet_number = 0;
        std::uint16_t other_player_count = 0;
        PlayerUuidLocation other_player[1];

        UserActionInternalAnswer()
        {
            type = EType::UserActionInternalAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct GetNodeInfo : public Base
    {
        GetNodeInfo()
        {
            type = EType::GetNodeInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct GetNodeInfoAnswer : public Base
    {
        bool success = false;
        SquareCell bounding_box;
        std::array<ip_address_t::bytes_type, 12> neighbor_addresses = { 0 };
        std::array<std::uint16_t, 12> neighbor_ports = { 0 };
        std::array<std::uint32_t, 12> neighbor_tokens = { 0 };
        ip_address_t::bytes_type parent_address = { 0 };
        unsigned short parent_port = 0;
        std::uint32_t parent_token = 0;

        GetNodeInfoAnswer()
        {
            type = EType::GetNodeInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct RegisterProxy : public Base
    {
        std::uint32_t proxy_index = 0;

        RegisterProxy()
        {
            type = EType::RegisterProxy;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct RegisterProxyAnswer : public Base
    {
        std::uint32_t proxy_index = 0;

        RegisterProxyAnswer()
        {
            type = EType::RegisterProxyAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct SpawnNodeServer : public Base
    {
        std::uint16_t node_server_port = 0;

        SpawnNodeServer()
        {
            type = EType::SpawnNodeServer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct SpawnNodeServerAnswer : public Base
    {
        SpawnNodeServerAnswer()
        {
            type = EType::SpawnNodeServerAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalancerServerInfo : public Base
    {
        MonitoringBalancerServerInfo()
        {
            type = EType::MonitoringBalancerServerInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalancerServerInfoAnswer : public Base
    {
        SquareCell global_bounding_box;
        std::uint32_t tree_root_token = 0;

        MonitoringBalancerServerInfoAnswer()
        {
            type = EType::MonitoringBalancerServerInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeInfo : public Base
    {
        std::uint32_t tree_node_token = 0;

        MonitoringBalanceTreeInfo()
        {
            type = EType::MonitoringBalanceTreeInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeInfoAnswer : public Base
    {
        bool success = false;
        std::uint32_t tree_node_token = 0;
        std::size_t level = 0;
        SquareCell bounding_box;
        bool leaf_node = true;
        std::array<std::uint32_t, CountOfChildren> children;
        std::uint32_t user_count = 0;
        ip_address_t::bytes_type node_server_address = { 0 };
        std::uint16_t node_server_port_number = 0;

        MonitoringBalanceTreeInfoAnswer()
        {
            type = EType::MonitoringBalanceTreeInfoAnswer;
            children.fill(0);
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeNeighborInfo : public Base
    {
        std::uint32_t tree_node_token = 0;
        CellIndex neighbor_cell;

        MonitoringBalanceTreeNeighborInfo()
        {
            type = EType::MonitoringBalanceTreeNeighborInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeNeighborInfoAnswer : public Base
    {
        bool success = false;
        std::uint32_t tree_node_token = 0;
        CellIndex neighbor_cell;
        std::uint32_t neighbor_node_token = 0;

        MonitoringBalanceTreeNeighborInfoAnswer()
        {
            type = EType::MonitoringBalanceTreeNeighborInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticSplit : public Base
    {
        std::uint32_t tree_node_token = 0;

        MonitoringBalanceTreeStaticSplit()
        {
            type = EType::MonitoringBalanceTreeStaticSplit;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticSplitAnswer : public Base
    {
        std::uint32_t tree_node_token = 0;
        bool success = false;
        bool not_leaf_node = false;
        bool node_server_running = false;

        MonitoringBalanceTreeStaticSplitAnswer()
        {
            type = EType::MonitoringBalanceTreeStaticSplitAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticMerge : public Base
    {
        std::uint32_t tree_node_token = 0;

        MonitoringBalanceTreeStaticMerge()
        {
            type = EType::MonitoringBalanceTreeStaticMerge;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticMergeAnswer : public Base
    {
        std::uint32_t tree_node_token = 0;
        bool success = false;

        MonitoringBalanceTreeStaticMergeAnswer()
        {
            type = EType::MonitoringBalanceTreeStaticMergeAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringGetProxyCount : public Base
    {
        MonitoringGetProxyCount()
        {
            type = EType::MonitoringGetProxyCount;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringGetProxyCountAnswer : public Base
    {
        std::uint32_t proxy_count = 0;

        MonitoringGetProxyCountAnswer()
        {
            type = EType::MonitoringGetProxyCountAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringGetProxyInfo : public Base
    {
        std::uint32_t proxy_index = 0;

        MonitoringGetProxyInfo()
        {
            type = EType::MonitoringGetProxyInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringGetProxyInfoAnswer : public Base
    {
        bool success = false;
        std::uint32_t proxy_index = 0;
        ip_address_t::bytes_type proxy_server_address = { 0 };
        std::uint16_t proxy_server_port_number = 0;

        MonitoringGetProxyInfoAnswer()
        {
            type = EType::MonitoringGetProxyInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringMessageCount : public Base
    {
        MonitoringMessageCount()
        {
            type = EType::MonitoringMessageCount;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringMessageCountAnswer : public Base
    {
        std::uint32_t message_count = 0;

        MonitoringMessageCountAnswer()
        {
            type = EType::MonitoringMessageCountAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringPopMessage : public Base
    {
        MonitoringPopMessage()
        {
            type = EType::MonitoringPopMessage;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringPopMessageAnswer : public Base
    {
        bool success = false;
        ESeverityType severity_type = ESeverityType::InfoMessage;
        EServerType server_type = EServerType::NodeServer;
        std::uint32_t token = 0;
        char message[256] = { 0 };
        std::uint32_t message_count = 0;

        MonitoringPopMessageAnswer()
        {
            type = EType::MonitoringPopMessageAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
        std::string getMessage() const
        {
            char temp_message[sizeof(message) / sizeof(message[0])];
            memcpy(temp_message, message, std::size(message));
            temp_message[std::size(temp_message) - 1] = 0;
            return temp_message;
        }
        void setMessage(const std::string& message_)
        {
            memset(message, 0, std::size(message));
            memcpy(message, message_.c_str(), std::min(message_.size(), std::size(message)));
        }
    };

    // I got this information about magic packet
    // from https://ru.wikipedia.org/wiki/Wake-on-LAN
    struct WakeOnLan
    {
        std::array<std::uint8_t, 6> synchronization_chain = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        union
        {
            std::array<Network::MacAddress, 16> mac_addresses = { 0 };
            std::uint32_t packet_number;
        };

        WakeOnLan()
        {
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }

        void setMacAddress(const Network::MacAddress& mac_address)
        {
            mac_addresses.fill(mac_address);
        }
    };

    inline size_t getSize(EType type)
    {
        switch (type)
        {
        case EType::Login:
            return sizeof(Login);
        case EType::LoginAnswer:
            return sizeof(LoginAnswer);
        case EType::Logout:
            return sizeof(Logout);
        case EType::LogoutAnswer:
            return sizeof(LogoutAnswer);
        case EType::LogoutInternal:
            return sizeof(LogoutInternal);
        case EType::LogoutInternalAnswer:
            return sizeof(LogoutInternalAnswer);
        case EType::InitializePosition:
            return sizeof(InitializePosition);
        case EType::InitializePositionAnswer:
            return sizeof(InitializePositionAnswer);
        case EType::InitializePositionInternal:
            return sizeof(InitializePositionInternal);
        case EType::InitializePositionInternalAnswer:
            return sizeof(InitializePositionInternalAnswer);
        case EType::UserAction:
            return sizeof(UserAction);
        case EType::UserActionAnswer:
            return sizeof(UserActionAnswer);
        case EType::UserActionInternal:
            return sizeof(UserActionInternal);
        case EType::UserActionInternalAnswer:
            return sizeof(UserActionInternalAnswer);
        case EType::GetNodeInfo:
            return sizeof(GetNodeInfo);
        case EType::GetNodeInfoAnswer:
            return sizeof(GetNodeInfoAnswer);
        case EType::RegisterProxy:
            return sizeof(RegisterProxy);
        case EType::RegisterProxyAnswer:
            return sizeof(RegisterProxyAnswer);
        case EType::SpawnNodeServer:
            return sizeof(SpawnNodeServer);
        case EType::SpawnNodeServerAnswer:
            return sizeof(SpawnNodeServerAnswer);
        case EType::MonitoringBalancerServerInfo:
            return sizeof(MonitoringBalancerServerInfo);
        case EType::MonitoringBalancerServerInfoAnswer:
            return sizeof(MonitoringBalancerServerInfoAnswer);
        case EType::MonitoringBalanceTreeInfo:
            return sizeof(MonitoringBalanceTreeInfo);
        case EType::MonitoringBalanceTreeInfoAnswer:
            return sizeof(MonitoringBalanceTreeInfoAnswer);
        case EType::MonitoringBalanceTreeNeighborInfo:
            return sizeof(MonitoringBalanceTreeNeighborInfo);
        case EType::MonitoringBalanceTreeNeighborInfoAnswer:
            return sizeof(MonitoringBalanceTreeNeighborInfoAnswer);
        case EType::MonitoringBalanceTreeStaticSplit:
            return sizeof(MonitoringBalanceTreeStaticSplit);
        case EType::MonitoringBalanceTreeStaticSplitAnswer:
            return sizeof(MonitoringBalanceTreeStaticSplitAnswer);
        case EType::MonitoringBalanceTreeStaticMerge:
            return sizeof(MonitoringBalanceTreeStaticMerge);
        case EType::MonitoringBalanceTreeStaticMergeAnswer:
            return sizeof(MonitoringBalanceTreeStaticMergeAnswer);
        case EType::MonitoringGetProxyCount:
            return sizeof(MonitoringGetProxyCount);
        case EType::MonitoringGetProxyCountAnswer:
            return sizeof(MonitoringGetProxyCountAnswer);
        case EType::MonitoringGetProxyInfo:
            return sizeof(MonitoringGetProxyInfo);
        case EType::MonitoringGetProxyInfoAnswer:
            return sizeof(MonitoringGetProxyInfoAnswer);
        case EType::MonitoringMessageCount:
            return sizeof(MonitoringMessageCount);
        case EType::MonitoringMessageCountAnswer:
            return sizeof(MonitoringMessageCountAnswer);
        case EType::MonitoringPopMessage:
            return sizeof(MonitoringPopMessage);
        case EType::MonitoringPopMessageAnswer:
            return sizeof(MonitoringPopMessageAnswer);
        default:
            return 0;
        }
    }

    template<class PacketType>
    inline size_t getSize(const PacketType* packet)
    {
        return sizeof(PacketType);
    }

    template<>
    inline size_t getSize<UserActionAnswer>(const UserActionAnswer* packet)
    {
        size_t other_user_count = std::max<size_t>(packet->other_player_count, 1);
        size_t result = sizeof(UserActionAnswer) + sizeof(PlayerUuidLocation) * (other_user_count - 1);
        assert(result <= MAX_SIZE);
        return result;
    }

    template<>
    inline size_t getSize<UserActionInternalAnswer>(const UserActionInternalAnswer* packet)
    {
        size_t other_user_count = std::max<size_t>(packet->other_player_count, 1);
        size_t result = sizeof(UserActionInternalAnswer) + sizeof(PlayerUuidLocation) * (other_user_count - 1);
        assert(result <= MAX_SIZE);
        return result;
    }
}

#pragma pack(pop)
