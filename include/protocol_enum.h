// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

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
}
