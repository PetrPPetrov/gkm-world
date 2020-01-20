// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <string>
#include <iostream>

namespace Packet
{
    enum class EType : std::uint8_t
    {
        // Client->Proxy protocol
        Login,
        LoginAnswer,
        Logout,
        LogoutAnswer,
        InitializePosition,
        InitializePositionAnswer,
        UserAction,
        UserActionAnswer,
        // Proxy->Balancer->Node protocol
        InitializePositionInternal,
        InitializePositionInternalAnswer,
        // Proxy->Node protocol
        LogoutInternal,
        LogoutInternalAnswer,
        // Proxy->Node protocol
        UserActionInternal,
        UserActionInternalAnswer,
        // Node->Node protocol
        UserMove, // Not used
        // Node->Proxy protocol
        AddNodeServerToProxy, // Not used
        RemoveNodeServerFromProxy, // Not used
        // Node->Balancer protocol
        GetNodeInfo,
        GetNodeInfoAnswer,
        // Balancer->Node protocol
        SpawnNodeServer, // Not used
        SpawnNodeServerAnswer, // Not used
        SplitNode, // Not used
        SplitNodeAnswer, // Not used
        // Markers
        MonitoringApi = 200,
        MonitoringBalancerServerInfo = MonitoringApi,
        MonitoringBalancerServerInfoAnswer,
        MonitoringBalanceTreeInfo,
        MonitoringBalanceTreeInfoAnswer,
        MonitoringBalanceTreeNeighborInfo,
        MonitoringBalanceTreeNeighborInfoAnswer,
        MonitoringBalanceTreeStaticSplit,
        MonitoringBalanceTreeStaticSplitAnswer,
        MonitoringBalanceTreeStaticMerge,
        MonitoringBalanceTreeStaticMergeAnswer,
        MonitoringMessageCount,
        MonitoringMessageCountAnswer,
        MonitoringPopMessage,
        MonitoringPopMessageAnswer,
        Last,
        First = Login
    };
    const std::uint8_t TYPE_FIRST = static_cast<std::uint8_t>(EType::First);
    const std::uint8_t TYPE_LAST = static_cast<std::uint8_t>(EType::Last);
    const std::uint8_t TYPE_COUNT = static_cast<std::uint8_t>(EType::Last);

    inline bool isGuaranteedDeliveryType(EType packet_type)
    {
        switch (packet_type)
        {
        case EType::UserAction:
        case EType::UserActionAnswer:
        case EType::UserActionInternal:
        case EType::UserActionInternalAnswer:
            return false;
        default:
            return true;
        }
    }

    enum class EServerType : std::uint8_t
    {
        ClientApplication,
        ProxyServer,
        NodeServer,
        BalancerServer,
        Monitor
    };

    inline std::string getText(EServerType server_type)
    {
        switch (server_type)
        {
        case EServerType::ClientApplication:
            return "[Client]";
        case EServerType::ProxyServer:
            return "[Proxy]";
        case EServerType::NodeServer:
            return "[Node]";
        case EServerType::BalancerServer:
            return "[Balancer]";
        case EServerType::Monitor:
            return "[Monitor]";
        default:
            return "[Unknown]";
        }
    }

    enum class ESeverityType : std::uint8_t
    {
        DebugMessage,
        InfoMessage,
        WarningMessage,
        ErrorMessage,
        FatalMessage
    };

    inline std::string getText(ESeverityType severity)
    {
        switch (severity)
        {
        case ESeverityType::DebugMessage:
            return "DEBUG";
        case ESeverityType::InfoMessage:
            return "INFO";
        case ESeverityType::WarningMessage:
            return "WARNING";
        case ESeverityType::ErrorMessage:
            return "ERROR";
        case ESeverityType::FatalMessage:
            return "FATAL";
        default:
            return "UNKNOWN_SEVERITY_TYPE";
        }
    }

    inline ESeverityType getSeverity(const std::string& string_representation)
    {
        if (string_representation == "INFO")
        {
            return ESeverityType::InfoMessage;
        }
        else if (string_representation == "WARNING")
        {
            return ESeverityType::WarningMessage;
        }
        else if (string_representation == "ERROR")
        {
            return ESeverityType::ErrorMessage;
        }
        else if (string_representation == "FATAL")
        {
            return ESeverityType::FatalMessage;
        }
        else
        {
            return ESeverityType::DebugMessage;
        }
    }

    inline std::istream& operator>>(std::istream& input_stream, ESeverityType& severity)
    {
        std::string string_representation;
        input_stream >> string_representation;
        severity = getSeverity(string_representation);
        return input_stream;
    }
}
