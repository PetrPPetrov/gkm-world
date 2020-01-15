// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <unordered_map>
#include <map>
#include <vector>
#include <chrono>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "protocol.h"
#include "packet_pool.h"
#include "log.h"

class Transport : private boost::noncopyable
{
protected:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::endpoint remote_end_point;
    boost::asio::ip::udp::socket socket;

private:
    char receive_buffer[Packet::MAX_SIZE] = { 0 };
    std::uint32_t cur_packet_number = 0;
    Packet::Pool<PACKET_POOL_SIZE> packet_pool;
    std::vector<std::function<bool(size_t)>> handlers;
    Packet::EServerType server_type = Packet::EServerType::BalancerServer;
    std::uint32_t server_token = 0;

    struct GuaranteedDeliveryInfo : public std::enable_shared_from_this<GuaranteedDeliveryInfo>
    {
        typedef GuaranteedDeliveryInfo SelfType;
        typedef std::shared_ptr<SelfType> Ptr;

        std::uint32_t packet_number = 0;
        std::uint8_t attempt_count = 8;
        boost::asio::deadline_timer timer;
        const boost::posix_time::milliseconds delay;
        boost::asio::ip::udp::endpoint end_point;
        void* packet_buffer = nullptr;
        std::size_t packet_buffer_size = 0;
        std::function<void()> error_handler;
        bool delivered_ok = false;

        GuaranteedDeliveryInfo(boost::asio::io_service& io_service, const boost::posix_time::milliseconds delay_) :
            timer(io_service, delay_), delay(delay_)
        {
        }
    };
    typedef std::unordered_map<std::uint32_t, GuaranteedDeliveryInfo::Ptr> guaranteed_delivery_map_t;
    guaranteed_delivery_map_t guaranteed_delivery_map;
    bool guaranteed_delivery_enabled = false;

public:
    Transport() : socket(io_service)
    {
        handlers.resize(Packet::TYPE_COUNT);
    }

    std::uint32_t getCurPacketNumber() const
    {
        return cur_packet_number;
    }

    void setReceiveHandler(Packet::EType type, std::function<bool(size_t)> handler)
    {
        handlers.at(static_cast<std::uint8_t>(type)) = handler;
    }

    void doReceive()
    {
        socket.async_receive_from(
            boost::asio::buffer(receive_buffer),
            remote_end_point,
            boost::bind(&Transport::onReceive, this, _1, _2));
    }

    template<class PacketType>
    PacketType* getReceiveBufferAs()
    {
        return reinterpret_cast<PacketType*>(receive_buffer);
    }

    template<class PacketType>
    PacketType* createPacket(std::uint32_t packet_number)
    {
        void* new_buffer = packet_pool.allocate();
        PacketType* result = new (new_buffer) PacketType();
        result->packet_number = packet_number;
        return result;
    }

    template<class PacketType>
    PacketType* createPacket()
    {
        return createPacket<PacketType>(++cur_packet_number);
    }

    template<class PacketType>
    void standardSendTo(const PacketType* packet, boost::asio::ip::udp::endpoint& end_point)
    {
#ifdef _DEBUG
        LOG_DEBUG << "standardSendTo " << end_point << " #" << packet->packet_number << std::endl;
#endif
        void* packet_buffer = static_cast<void*>(const_cast<PacketType*>(packet));
        socket.async_send_to(
            boost::asio::buffer(packet, getSize(packet)),
            end_point,
            boost::bind(&Transport::onSend, this, packet_buffer, _1, _2));
    }

    template<class PacketType>
    void standardSend(const PacketType* packet)
    {
        standardSendTo<PacketType>(packet, remote_end_point);
    }

    template<class PacketType, size_t AttemptCount = 10, size_t WaitMs = 1000>
    void guaranteedSendTo(const PacketType* packet, boost::asio::ip::udp::endpoint& end_point, std::function<void()> error_handler)
    {
#ifdef _DEBUG
        LOG_DEBUG << "guaranteedSendTo " << end_point << " #" << packet->packet_number << std::endl;
#endif
        assert(Packet::isGuaranteedDeliveryType(packet->type));
        guaranteed_delivery_enabled = true;
        void* packet_buffer = static_cast<void*>(const_cast<PacketType*>(packet));
        size_t packet_size = getSize(packet);
        socket.async_send_to(
            boost::asio::buffer(packet, packet_size),
            end_point,
            boost::bind(&Transport::onGuaranteedSend, this, packet_buffer, _1, _2));
        GuaranteedDeliveryInfo::ptr info = std::make_shared<GuaranteedDeliveryInfo>(io_service, boost::posix_time::milliseconds(WaitMs));
        info->packet_number = packet->packet_number;
        info->attempt_count = AttemptCount;
        info->packet_buffer = packet_buffer;
        info->packet_buffer_size = packet_size;
        info->end_point = end_point;
        info->error_handler = error_handler;
        guaranteed_delivery_map[packet->packet_number] = info;
        info->timer.async_wait(boost::bind(&Transport::onTimeout, this, info, _1));
    }

    template<class PacketType>
    void guaranteedSend(const PacketType* packet, std::function<void()> error_handler)
    {
        guaranteedSendTo<PacketType>(packet, remote_end_point, error_handler);
    }

    void setServerType(Packet::EServerType server_type_)
    {
        server_type = server_type_;
    }

    void setServerToken(std::uint32_t server_token_)
    {
        server_token = server_token_;
    }

private:
    void onReceive(const boost::system::error_code& error, size_t received_bytes)
    {
        if (error == boost::asio::error::connection_reset)
        {
#ifdef _DEBUG
            LOG_DEBUG << "error && error != boost::asio::error::connection_reset, size = " << received_bytes << std::endl;
#endif
            doReceive();
            return;
        }

        if (error && error != boost::asio::error::message_size)
        {
#ifdef _DEBUG
            LOG_DEBUG << "error && error != boost::asio::error::message_size, size = " << received_bytes << std::endl;
#endif
            return;
        }

        if (received_bytes < sizeof(Packet::Base))
        {
#ifdef _DEBUG
            LOG_DEBUG << "received_bytes < sizeof(Packet::Base)" << std::endl;
#endif
            return;
        }

#ifdef _DEBUG
        LOG_DEBUG << "received_bytes " << received_bytes << std::endl;
#endif

        auto basic_packet = reinterpret_cast<const Packet::Base*>(receive_buffer);
        std::uint8_t type_code = static_cast<std::uint8_t>(basic_packet->type);
        if (type_code < Packet::TYPE_FIRST || type_code >= Packet::TYPE_LAST)
        {
#ifdef _DEBUG
            LOG_DEBUG << "unknown packet type, packet type = " << static_cast<unsigned>(type_code) << std::endl;
#endif
            return;
        }

        const size_t minimum_size = Packet::getSize(basic_packet->type);
        if (received_bytes < minimum_size)
        {
#ifdef _DEBUG
            LOG_DEBUG << "received_bytes < " << minimum_size << std::endl;
#endif
            return;
        }

        if (guaranteed_delivery_enabled && Packet::isGuaranteedDeliveryType(basic_packet->type))
        {
            onGuaranteedAnswer(basic_packet->packet_number);
        }

        bool handler_return_value = true;
        std::function<bool(size_t)>& handler = handlers[type_code];
        if (handler)
        {
            handler_return_value = handler(received_bytes);
        }
        else
        {
#ifdef _DEBUG
            LOG_DEBUG << "unknown packet type, packet type = " << static_cast<unsigned>(type_code) << std::endl;
#endif
        }

        if (handler_return_value)
        {
            doReceive();
        }
    }

    void onGuaranteedAnswer(std::uint32_t packet_number)
    {
        guaranteed_delivery_map_t::iterator fit = guaranteed_delivery_map.find(packet_number);
        if (fit != guaranteed_delivery_map.end())
        {
#ifdef _DEBUG
            LOG_DEBUG << "found guaranteed request #" << packet_number << std::endl;
#endif
            fit->second->delivered_ok = true;
            fit->second->timer.cancel();
            packet_pool.deallocate(fit->second->packet_buffer);
            guaranteed_delivery_map.erase(fit);
        }
        else
        {
#ifdef _DEBUG
            LOG_WARNING << "guaranteed request is not found #" << packet_number << std::endl;
#endif
        }
    }

    void onSend(void* packet_buffer, const boost::system::error_code& error, size_t transferred_bytes)
    {
#ifdef _DEBUG
        Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(packet_buffer);
        LOG_DEBUG << "onSend #" << basic_packet->packet_number << std::endl;
#endif
        packet_pool.deallocate(packet_buffer);
    }

    void onGuaranteedSend(void* packet_buffer, const boost::system::error_code& error, size_t transferred_bytes)
    {
#ifdef _DEBUG
        Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(packet_buffer);
        LOG_DEBUG << "onGuaranteedSend #" << basic_packet->packet_number << std::endl;
#endif
    }

    void onTimeout(GuaranteedDeliveryInfo::Ptr info, const boost::system::error_code& error)
    {
        if (info->delivered_ok)
        {
#ifdef _DEBUG
            Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(info->packet_buffer);
            LOG_INFO << "guaranteed delivered OK #" << info->packet_number << std::endl;
#endif
            info->timer.cancel();
            guaranteed_delivery_map.erase(info->packet_number);
            return;
        }
        if (info->attempt_count)
        {
#ifdef _DEBUG
            Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(info->packet_buffer);
            LOG_WARNING << "guaranteed failed to delivery, new attempt #" << basic_packet->packet_number << std::endl;
#endif
            info->attempt_count--;
            socket.async_send_to(
                boost::asio::buffer(info->packet_buffer, info->packet_buffer_size),
                info->end_point,
                boost::bind(&Transport::onGuaranteedSend, this, info->packet_buffer, _1, _2));
            info->timer.expires_at(info->timer.expires_at() + info->delay);
            info->timer.async_wait(boost::bind(&Transport::onTimeout, this, info, _1));
        }
        else
        {
#ifdef _DEBUG
            Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(info->packet_buffer);
            LOG_ERROR << "guaranteeed failed to devilery #" << basic_packet->packet_number << std::endl;
#endif
            info->timer.cancel();
            packet_pool.deallocate(info->packet_buffer);
            guaranteed_delivery_map.erase(info->packet_number);
            if (info->error_handler)
            {
                info->error_handler();
            }
        }
    }

    bool onMonitoringMessageCount(size_t received_bytes)
    {
#ifdef _DEBUG
        LOG_DEBUG << "onMonitoringMessageCount";
#endif

        const auto packet = getReceiveBufferAs<Packet::MonitoringMessageCount>();
        auto answer = createPacket<Packet::MonitoringMessageCountAnswer>(packet->packet_number);
        answer->message_count = static_cast<std::uint32_t>(g_logger->messages.size());
        standardSend(answer);
        return true;
    }

    bool onMonitoringPopMessage(size_t received_bytes)
    {
#ifdef _DEBUG
        LOG_DEBUG << "onMonitoringPopMessage";
#endif

        const auto packet = getReceiveBufferAs<Packet::MonitoringPopMessage>();
        auto answer = createPacket<Packet::MonitoringPopMessageAnswer>(packet->packet_number);
        answer->server_type = server_type;
        answer->token = server_token;

        if (g_logger->messages.size() > 0)
        {
            Log::Message message = g_logger->messages.front();
            answer->success = true;
            answer->severity_type = message.severity;
            answer->setMessage(message.text);
            g_logger->messages.pop_front();
        }
        else
        {
            answer->success = false;
        }
        standardSend(answer);
        return true;
    }
};
