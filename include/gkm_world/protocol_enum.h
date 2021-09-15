// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <string>
#include <iostream>

namespace Packet {
    enum class EType : std::uint8_t {
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
        UnitAction,                                   // Client->Proxy
        UnitActionAnswer,                             // Proxy->Client
        UnitActionInternal,                           // Proxy->Node
        UnitActionInternalAnswer,                     // Node->Proxy
        GetNodeInfo,                                  // Node->Balancer
        GetNodeInfoAnswer,                            // Balancer->Node
        RegisterProxy,                                // Proxy->Balancer
        RegisterProxyAnswer,                          // Balancer->Proxy
        SpawnNodeServer,                              // Balancer->Node, not used
        SpawnNodeServerAnswer,                        // Node->Balancer, not used
        // Logging
        LogMessage = 150,                             // {Proxy | Node | Balancer}->Log
        LogMessageAnswer,                             // Log->{Proxy | Node | Balancer}
        LogGetMessage,                                // Monitor->Log
        LogGetMessageAnswer,                          // Log->Monitor
        // Monitoring
        MonitoringBalancerServerInfo = 200,           // Monitor->Balancer
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
        Last,
        First = Login
    };
    const std::uint8_t TYPE_FIRST = static_cast<std::uint8_t>(EType::First);
    const std::uint8_t TYPE_LAST = static_cast<std::uint8_t>(EType::Last);
    const std::uint8_t TYPE_COUNT = static_cast<std::uint8_t>(EType::Last);

    inline bool isGuaranteedDeliveryType(EType packet_type) {
        switch (packet_type) {
        case EType::UnitAction:
        case EType::UnitActionAnswer:
        case EType::UnitActionInternal:
        case EType::UnitActionInternalAnswer:
            return false;
        default:
            return true;
        }
    }

    inline std::string getText(EType packet_type) {
        switch (packet_type) {
        case EType::Login:
            return "Login";
        case EType::LoginAnswer:
            return "LoginAnswer";
        case EType::Logout:
            return "Logout";
        case EType::LogoutAnswer:
            return "LogoutAnswer";
        case EType::LogoutInternal:
            return "LogoutInternal";
        case EType::LogoutInternalAnswer:
            return "LogoutInternalAnswer";
        case EType::InitializePosition:
            return "InitializePosition";
        case EType::InitializePositionAnswer:
            return "InitializePositionAnswer";
        case EType::InitializePositionInternal:
            return "InitializePositionInternal";
        case EType::InitializePositionInternalAnswer:
            return "InitializePositionInternalAnswer";
        case EType::UnitAction:
            return "UnitAction";
        case EType::UnitActionAnswer:
            return "UnitActionAnswer";
        case EType::UnitActionInternal:
            return "UnitActionInternal";
        case EType::UnitActionInternalAnswer:
            return "UnitActionInternalAnswer";
        case EType::GetNodeInfo:
            return "GetNodeInfo";
        case EType::GetNodeInfoAnswer:
            return "GetNodeInfoAnswer";
        case EType::RegisterProxy:
            return "RegisterProxy";
        case EType::RegisterProxyAnswer:
            return "RegisterProxyAnswer";
        case EType::SpawnNodeServer:
            return "SpawnNodeServer";
        case EType::SpawnNodeServerAnswer:
            return "SpawnNodeServerAnswer";
        case EType::LogMessage:
            return "LogMessage";
        case EType::LogMessageAnswer:
            return "LogMessageAnswer";
        case EType::LogGetMessage:
            return "LogGetMessage";
        case EType::LogGetMessageAnswer:
            return "LogGetMessageAnswer";
        case EType::MonitoringBalancerServerInfo:
            return "MonitoringBalancerServerInfo";
        case EType::MonitoringBalancerServerInfoAnswer:
            return "MonitoringBalancerServerInfoAnswer";
        case EType::MonitoringBalanceTreeInfo:
            return "MonitoringBalanceTreeInfo";
        case EType::MonitoringBalanceTreeInfoAnswer:
            return "MonitoringBalanceTreeInfoAnswer";
        case EType::MonitoringBalanceTreeNeighborInfo:
            return "MonitoringBalanceTreeNeighborInfo";
        case EType::MonitoringBalanceTreeNeighborInfoAnswer:
            return "MonitoringBalanceTreeNeighborInfoAnswer";
        case EType::MonitoringBalanceTreeStaticSplit:
            return "MonitoringBalanceTreeStaticSplit";
        case EType::MonitoringBalanceTreeStaticSplitAnswer:
            return "MonitoringBalanceTreeStaticSplitAnswer";
        case EType::MonitoringBalanceTreeStaticMerge:
            return "MonitoringBalanceTreeStaticMerge";
        case EType::MonitoringBalanceTreeStaticMergeAnswer:
            return "MonitoringBalanceTreeStaticMergeAnswer";
        case EType::MonitoringGetProxyCount:
            return "MonitoringGetProxyCount";
        case EType::MonitoringGetProxyCountAnswer:
            return "MonitoringGetProxyCountAnswer";
        case EType::MonitoringGetProxyInfo:
            return "MonitoringGetProxyInfo";
        case EType::MonitoringGetProxyInfoAnswer:
            return "MonitoringGetProxyInfoAnswer";
        default:
            return "Unknown";
        }
    }

    enum class EApplicationType : std::uint8_t {
        ClientApplication,
        ProxyServer,
        NodeServer,
        BalancerServer,
        LogServer,
        ResourceServer,
        MonitorApplication
    };

    inline std::string getText(EApplicationType application_type) {
        switch (application_type) {
        case EApplicationType::ClientApplication:
            return "[Client]";
        case EApplicationType::ProxyServer:
            return "[Proxy]";
        case EApplicationType::NodeServer:
            return "[Node]";
        case EApplicationType::BalancerServer:
            return "[Balancer]";
        case EApplicationType::LogServer:
            return "[Log]";
        case EApplicationType::ResourceServer:
            return "[Resource]";
        case EApplicationType::MonitorApplication:
            return "[Monitor]";
        default:
            return "[Unknown]";
        }
    }

    enum class ESeverityType : std::uint8_t {
        DebugMessage,
        InfoMessage,
        WarningMessage,
        ErrorMessage,
        FatalMessage
    };

    inline std::string getText(ESeverityType severity) {
        switch (severity) {
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

    inline ESeverityType getSeverity(const std::string& string_representation) {
        if (string_representation == "INFO") {
            return ESeverityType::InfoMessage;
        } else if (string_representation == "WARNING") {
            return ESeverityType::WarningMessage;
        } else if (string_representation == "ERROR") {
            return ESeverityType::ErrorMessage;
        } else if (string_representation == "FATAL") {
            return ESeverityType::FatalMessage;
        } else {
            return ESeverityType::DebugMessage;
        }
    }

    inline std::istream& operator>>(std::istream& input_stream, ESeverityType& severity) {
        std::string string_representation;
        input_stream >> string_representation;
        severity = getSeverity(string_representation);
        return input_stream;
    }
}
