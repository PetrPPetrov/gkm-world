// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <string>
#include <iostream>

namespace Packet
{
    enum class EType : std::uint8_t
    {
        Login,                                        // Client->Proxy
        LoginAnswer,                                  // Proxy->Client
        Logout,                                       // Client->Proxy
        LogoutAnswer,                                 // Proxy->Client
        LogoutInternal,                               // Proxy->Node
        LogoutInternalAnswer,                         // Node->Proxy
        InitializePosition,                           // Client->Proxy
        InitializePositionAnswer,                     // Proxy->Client
        InitializePositionInternal,                   // Proxy->Balancer, Balancer->Node
        InitializePositionInternalAnswer,             // Node->Proxy
        UserAction,                                   // Client->Proxy
        UserActionAnswer,                             // Proxy->Client
        UserActionInternal,                           // Proxy->Node
        UserActionInternalAnswer,                     // Node->Proxy
        GetNodeInfo,                                  // Node->Balancer
        GetNodeInfoAnswer,                            // Balancer->Node
        RegisterProxy,                                // Proxy->Balancer
        RegisterProxyAnswer,                          // Balancer->Proxy
        SpawnNodeServer,                              // Balancer->Node, not used
        SpawnNodeServerAnswer,                        // Node->Balancer, not used
        // Markers
        MonitoringApi = 200,
        MonitoringBalancerServerInfo = MonitoringApi, // Monitor->Balancer
        MonitoringBalancerServerInfoAnswer,           // Balancer->Monitor
        MonitoringBalanceTreeInfo,                    // Monitor->Balancer
        MonitoringBalanceTreeInfoAnswer,              // Balancer->Monitor
        MonitoringBalanceTreeNeighborInfo,            // Monitor->Balancer
        MonitoringBalanceTreeNeighborInfoAnswer,      // Balancer->Monitor
        MonitoringBalanceTreeStaticSplit,             // Monitor->Balancer
        MonitoringBalanceTreeStaticSplitAnswer,       // Balancer->Monitor
        MonitoringBalanceTreeStaticMerge,             // Monitor->Balancer
        MonitoringBalanceTreeStaticMergeAnswer,       // Balancer->Monitor
        MonitoringGetProxyCount,                      // Monitor->Balancer
        MonitoringGetProxyCountAnswer,                // Balancer->Monitor
        MonitoringGetProxyInfo,                       // Monitor->Balancer
        MonitoringGetProxyInfoAnswer,                 // Balancer->Monitor
        MonitoringMessageCount,                       // Monitor->Balancer, Monitor->Proxy, Monitor->Node
        MonitoringMessageCountAnswer,                 // Balancer->Monitor, Proxy->Monitor, Node->Monitor
        MonitoringPopMessage,                         // Monitor->Balancer, Monitor->Proxy, Monitor->Node
        MonitoringPopMessageAnswer,                   // Balancer->Monitor, Proxy->Monitor, Node->Monitor
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
