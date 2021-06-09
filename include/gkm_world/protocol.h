// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include "gkm_world/gkm_world.h"
#include "gkm_world/protocol_enum.h"
#include "gkm_world/fixed_string.h"
#include "gkm_world/mac_address.h"
#include "gkm_world/balance_tree/common.h"

typedef boost::asio::ip::address_v4 IpAddress;
#define TO_V to_v4

#pragma pack(push, 1)

namespace Packet
{
    static const std::size_t MAX_SIZE = 1024;
    static const std::uint16_t MAX_UNIT_COUNT_IN_PACKET = 20;

    typedef std::array<std::uint8_t, MAX_SIZE> MaxPacketArrayType;

    struct ProtocolCoordinate2D
    {
        CoordinateType x = 0;
        CoordinateType y = 0;
    };

    struct ProtocolUnitLocation
    {
        ProtocolCoordinate2D position;
        AngularType direction = 0;
    };

    struct ProtocolUnitLocationToken : public ProtocolUnitLocation
    {
        IndexType unit_token;
    };

    constexpr std::uint8_t UP_KEY_MASK = 0x01;
    constexpr std::uint8_t DOWN_KEY_MASK = 0x02;
    constexpr std::uint8_t LEFT_KEY_MASK = 0x04;
    constexpr std::uint8_t RIGHT_KEY_MASK = 0x08;

    struct ProtocolKeyboardState
    {
        std::uint8_t state = 0;
    };

    struct ProtocolSquare2D
    {
        ProtocolCoordinate2D start;
        CoordinateType size;
    };

    struct Base
    {
        EType type;
        IndexType packet_number = 0;
        IndexType buffer_index = INVALID_INDEX;
    };

    struct Login : public Base
    {
        String255 login;
        String255 password;
        String255 full_name;

        Login()
        {
            type = EType::Login;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LoginAnswer : public Base
    {
        bool success = false;
        IndexType unit_token = 0;

        LoginAnswer()
        {
            type = EType::LoginAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct Logout : public Base
    {
        IndexType unit_token = 0;

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
        IndexType client_packet_number = 0;
        IndexType unit_token = 0;

        LogoutInternal()
        {
            type = EType::LogoutInternal;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogoutInternalAnswer : public Base
    {
        bool success = false;
        IndexType unit_token = 0;
        IndexType client_packet_number = 0;

        LogoutInternalAnswer()
        {
            type = EType::LogoutInternalAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePosition : public Base
    {
        ProtocolUnitLocationToken unit_location;

        InitializePosition()
        {
            type = EType::InitializePosition;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePositionAnswer : public Base
    {
        bool success = false;
        ProtocolUnitLocation corrected_location;

        InitializePositionAnswer()
        {
            type = EType::InitializePositionAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePositionInternal : public Base
    {
        ProtocolUnitLocationToken unit_location;
        IndexType client_packet_number = 0;
        IndexType proxy_packet_number = 0;
        IpAddress::bytes_type proxy_server_address = { 0 };
        PortNumberType proxy_server_port_number = 0;

        InitializePositionInternal()
        {
            type = EType::InitializePositionInternal;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct InitializePositionInternalAnswer : public Base
    {
        bool success = false;
        IpAddress::bytes_type node_server_address = { 0 };
        PortNumberType node_server_port_number = 0;
        ProtocolUnitLocationToken corrected_location;
        IndexType client_packet_number = 0;
        IndexType proxy_packet_number = 0;

        InitializePositionInternalAnswer()
        {
            type = EType::InitializePositionInternalAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UnitAction : public Base
    {
        IndexType unit_token = 0;
        ProtocolKeyboardState keyboard_state;

        UnitAction()
        {
            type = EType::UnitAction;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UnitActionAnswer : public Base
    {
        ProtocolUnitLocation unit_location;
        std::uint16_t other_unit_count = 0;
        ProtocolUnitLocationToken other_unit[1];

        UnitActionAnswer()
        {
            type = EType::UnitActionAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UnitActionInternal : public Base
    {
        IndexType unit_token = 0;
        ProtocolKeyboardState keyboard_state;
        IndexType client_packet_number = 0;

        UnitActionInternal()
        {
            type = EType::UnitActionInternal;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct UnitActionInternalAnswer : public Base
    {
        ProtocolUnitLocationToken unit_location;
        IndexType client_packet_number = 0;
        std::uint16_t other_unit_count = 0;
        ProtocolUnitLocationToken other_unit[1];

        UnitActionInternalAnswer()
        {
            type = EType::UnitActionInternalAnswer;
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
        ProtocolSquare2D bounding_box;
        std::array<IpAddress::bytes_type, 12> neighbor_addresses = { 0 };
        std::array<PortNumberType, 12> neighbor_ports = { 0 };
        std::array<IndexType, 12> neighbor_tokens = { 0 };
        IpAddress::bytes_type parent_address = { 0 };
        PortNumberType parent_port = 0;
        IndexType parent_token = 0;

        GetNodeInfoAnswer()
        {
            type = EType::GetNodeInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct RegisterProxy : public Base
    {
        IndexType proxy_token = 0;

        RegisterProxy()
        {
            type = EType::RegisterProxy;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct RegisterProxyAnswer : public Base
    {
        IndexType proxy_token = 0;

        RegisterProxyAnswer()
        {
            type = EType::RegisterProxyAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct SpawnNodeServer : public Base
    {
        PortNumberType node_server_port = 0;

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

    struct LogMessage : public Base
    {
        EApplicationType application_type = EApplicationType::NodeServer;
        IndexType application_token = 0;
        ESeverityType severity_type = ESeverityType::DebugMessage;
        String512 message;

        LogMessage()
        {
            type = EType::LogMessage;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogMessageAnswer : public Base
    {
        LogMessageAnswer()
        {
            type = EType::LogMessageAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogGetMessage : public Base
    {
        IndexType message_token = 0;
        bool remove_message = false;

        LogGetMessage()
        {
            type = EType::LogGetMessage;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct LogGetMessageAnswer : public Base
    {
        IndexType message_token = 0;
        IndexType message_count = 0;
        EApplicationType server_type = EApplicationType::NodeServer;
        IndexType server_token = 0;
        ESeverityType severity_type = ESeverityType::DebugMessage;
        String512 message;

        LogGetMessageAnswer()
        {
            type = EType::LogGetMessageAnswer;
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
        ProtocolSquare2D global_bounding_box;
        IndexType tree_root_token = 0;

        MonitoringBalancerServerInfoAnswer()
        {
            type = EType::MonitoringBalancerServerInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeInfo : public Base
    {
        IndexType tree_node_token = 0;

        MonitoringBalanceTreeInfo()
        {
            type = EType::MonitoringBalanceTreeInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeInfoAnswer : public Base
    {
        bool success = false;
        IndexType tree_node_token = 0;
        std::uint16_t level = 0;
        ProtocolSquare2D bounding_box;
        bool leaf_node = true;
        std::array<IndexType, CountOfChildren> children;
        IndexType unit_count = 0;
        IpAddress::bytes_type node_server_address = { 0 };
        PortNumberType node_server_port_number = 0;

        MonitoringBalanceTreeInfoAnswer()
        {
            type = EType::MonitoringBalanceTreeInfoAnswer;
            children.fill(0);
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeNeighborInfo : public Base
    {
        IndexType tree_node_token = 0;
        ProtocolCoordinate2D neighbor_cell;

        MonitoringBalanceTreeNeighborInfo()
        {
            type = EType::MonitoringBalanceTreeNeighborInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeNeighborInfoAnswer : public Base
    {
        bool success = false;
        IndexType tree_node_token = 0;
        ProtocolCoordinate2D neighbor_cell;
        IndexType neighbor_node_token = 0;

        MonitoringBalanceTreeNeighborInfoAnswer()
        {
            type = EType::MonitoringBalanceTreeNeighborInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticSplit : public Base
    {
        IndexType tree_node_token = 0;

        MonitoringBalanceTreeStaticSplit()
        {
            type = EType::MonitoringBalanceTreeStaticSplit;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticSplitAnswer : public Base
    {
        IndexType tree_node_token = 0;
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
        IndexType tree_node_token = 0;

        MonitoringBalanceTreeStaticMerge()
        {
            type = EType::MonitoringBalanceTreeStaticMerge;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringBalanceTreeStaticMergeAnswer : public Base
    {
        IndexType tree_node_token = 0;
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
        IndexType proxy_count = 0;

        MonitoringGetProxyCountAnswer()
        {
            type = EType::MonitoringGetProxyCountAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringGetProxyInfo : public Base
    {
        IndexType proxy_index = 0;

        MonitoringGetProxyInfo()
        {
            type = EType::MonitoringGetProxyInfo;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
        }
    };

    struct MonitoringGetProxyInfoAnswer : public Base
    {
        bool success = false;
        IndexType proxy_index = 0;
        IpAddress::bytes_type proxy_server_address = { 0 };
        PortNumberType proxy_server_port_number = 0;

        MonitoringGetProxyInfoAnswer()
        {
            type = EType::MonitoringGetProxyInfoAnswer;
            static_assert(MAX_SIZE > sizeof(*this), "packet size exceeds the maximum allowed size");
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
        case EType::UnitAction:
            return sizeof(UnitAction);
        case EType::UnitActionAnswer:
            return sizeof(UnitActionAnswer);
        case EType::UnitActionInternal:
            return sizeof(UnitActionInternal);
        case EType::UnitActionInternalAnswer:
            return sizeof(UnitActionInternalAnswer);
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
        case EType::LogMessage:
            return sizeof(LogMessage);
        case EType::LogMessageAnswer:
            return sizeof(LogMessageAnswer);
        case EType::LogGetMessage:
            return sizeof(LogGetMessage);
        case EType::LogGetMessageAnswer:
            return sizeof(LogGetMessageAnswer);
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
        default:
            return 0;
        }
    }

    template<class PacketType>
    inline std::size_t getSize(const PacketType* packet)
    {
        return sizeof(PacketType);
    }

    template<>
    inline std::size_t getSize<UnitActionAnswer>(const UnitActionAnswer* packet)
    {
        std::size_t other_unit_count = std::max<std::size_t>(packet->other_unit_count, 1);
        std::size_t result = sizeof(UnitActionAnswer) + sizeof(ProtocolUnitLocationToken) * (other_unit_count - 1);
        assert(result <= MAX_SIZE);
        return result;
    }

    template<>
    inline std::size_t getSize<UnitActionInternalAnswer>(const UnitActionInternalAnswer* packet)
    {
        std::size_t other_unit_count = std::max<size_t>(packet->other_unit_count, 1);
        std::size_t result = sizeof(UnitActionInternalAnswer) + sizeof(ProtocolUnitLocationToken) * (other_unit_count - 1);
        assert(result <= MAX_SIZE);
        return result;
    }

    template<>
    inline std::size_t getSize<LogMessage>(const LogMessage* packet)
    {
        const std::size_t result = sizeof(LogMessage);
        const std::size_t unused_space = packet->message.unusedSpace();
        return result - unused_space;
    }

    template<>
    inline std::size_t getSize<LogGetMessageAnswer>(const LogGetMessageAnswer* packet)
    {
        const std::size_t result = sizeof(LogGetMessageAnswer);
        const std::size_t unused_space = packet->message.unusedSpace();
        return result - unused_space;
    }
}

#pragma pack(pop)
