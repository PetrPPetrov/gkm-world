// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include <chrono>
#include <functional>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "gkm_world/protocol.h"
#include "gkm_world/fast_index.h"
#include "gkm_world/logger.h"

class Transport
{
protected:
    boost::asio::io_service io_service;
    boost::asio::deadline_timer logging_timer;
    boost::asio::ip::udp::endpoint remote_end_point;
    boost::asio::ip::udp::socket socket;

private:
    char receive_buffer[Packet::MAX_SIZE] = { 0 };
    IndexType cur_packet_number = 0;
    Memory::FastIndexChain<Packet::MaxPacketArrayType> packet_pool;
    std::vector<std::function<bool(size_t)>> handlers;
    Packet::EApplicationType application_type = Packet::EApplicationType::BalancerServer;
    IndexType application_token = 0;

    struct GuaranteedDeliveryInfo : public std::enable_shared_from_this<GuaranteedDeliveryInfo>
    {
        typedef std::shared_ptr<GuaranteedDeliveryInfo> Ptr;

        std::uint32_t packet_number = 0;
        std::uint8_t attempt_count = 8;
        boost::asio::deadline_timer timer;
        const boost::posix_time::milliseconds delay;
        boost::asio::ip::udp::endpoint end_point;
        void* packet_buffer = nullptr;
        IndexType packet_buffer_index = INVALID_INDEX;
        std::size_t packet_buffer_size = 0;
        std::function<void(void*, std::size_t)> error_handler;
        bool delivered_ok = false;

        GuaranteedDeliveryInfo(boost::asio::io_service& io_service, const boost::posix_time::milliseconds delay_) :
            timer(io_service, delay_), delay(delay_)
        {
        }
    };
    typedef std::unordered_map<std::uint32_t, GuaranteedDeliveryInfo::Ptr> GuaranteedDeliveryMap;
    GuaranteedDeliveryMap guaranteed_delivery_map;
    bool guaranteed_delivery_enabled = false;

public:
    Transport(const Transport&) = delete;
    Transport& operator=(const Transport&) = delete;

    Transport() : socket(io_service), logging_timer(io_service, LOGGING_TIME_INTERVAL)
    {
        handlers.resize(Packet::TYPE_COUNT);
        logging_timer.async_wait(boost::bind(&Transport::onLoggingTimer, this, _1));
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
    PacketType* createPacket(IndexType packet_number)
    {
        IndexType result_index;
        void* new_buffer = packet_pool.chainPushBack(result_index);
        PacketType* result = new (new_buffer) PacketType();
        result->packet_number = packet_number;
        result->buffer_index = result_index;
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
        LOG_DEBUG << "standardSendTo " << end_point << " #" << packet->packet_number;

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
    void guaranteedSendTo(const PacketType* packet, boost::asio::ip::udp::endpoint& end_point, std::function<void(void*, std::size_t)> error_handler)
    {
        LOG_DEBUG << "guaranteedSendTo " << end_point << " #" << packet->packet_number;

        assert(Packet::isGuaranteedDeliveryType(packet->type));
        guaranteed_delivery_enabled = true;
        void* packet_buffer = static_cast<void*>(const_cast<PacketType*>(packet));
        size_t packet_size = getSize(packet);
        socket.async_send_to(
            boost::asio::buffer(packet, packet_size),
            end_point,
            boost::bind(&Transport::onGuaranteedSend, this, packet_buffer, _1, _2));
        GuaranteedDeliveryInfo::Ptr info = std::make_shared<GuaranteedDeliveryInfo>(io_service, boost::posix_time::milliseconds(WaitMs));
        info->packet_number = packet->packet_number;
        info->attempt_count = AttemptCount;
        info->packet_buffer = packet_buffer;
        info->packet_buffer_index = packet->buffer_index;
        info->packet_buffer_size = packet_size;
        info->end_point = end_point;
        info->error_handler = error_handler;
        guaranteed_delivery_map[packet->packet_number] = info;
        info->timer.async_wait(boost::bind(&Transport::onTimeout, this, info, _1));
    }

    template<class PacketType>
    void guaranteedSend(const PacketType* packet, std::function<void(void*, std::size_t)> error_handler)
    {
        guaranteedSendTo<PacketType>(packet, remote_end_point, error_handler);
    }

    void setApplicationType(Packet::EApplicationType application_type_)
    {
        application_type = application_type_;
    }

    void setApplicationToken(IndexType token)
    {
        application_token = token;
    }

    void logError(void* packet_buffer, std::size_t packet_buffer_size)
    {
        if (packet_buffer_size >= sizeof(Packet::Base))
        {
            auto base_packet = reinterpret_cast<const Packet::Base*>(packet_buffer);
            LOG_ERROR << "can not delivery packet; type " << getText(base_packet->type) << " length " << base_packet->packet_number;
        }
    }

    void silentlyIgnore(void* /*packet_buffer*/, std::size_t /*packet_buffer_size*/)
    {
    }

private:
    void onLoggingTimer(const boost::system::error_code& error)
    {
        if (error)
        {
            LOG_ERROR << "onLoggingTimer error";
            return;
        }

        Logger::processNextQueuedMessage();

        logging_timer.expires_at(logging_timer.expires_at() + LOGGING_TIME_INTERVAL);
        logging_timer.async_wait(boost::bind(&Transport::onLoggingTimer, this, _1));
    }

    void onReceive(const boost::system::error_code& error, size_t received_bytes)
    {
        if (error == boost::asio::error::connection_reset)
        {
            LOG_DEBUG << "error && error != boost::asio::error::connection_reset, size = " << received_bytes;
            doReceive();
            return;
        }

        if (error && error != boost::asio::error::message_size)
        {
            LOG_DEBUG << "error && error != boost::asio::error::message_size, size = " << received_bytes;
            return;
        }

        if (received_bytes < sizeof(Packet::Base))
        {
            LOG_DEBUG << "received_bytes < sizeof(Packet::Base)";
            return;
        }

        LOG_DEBUG << "received_bytes " << received_bytes;

        auto basic_packet = reinterpret_cast<const Packet::Base*>(receive_buffer);
        std::uint8_t type_code = static_cast<std::uint8_t>(basic_packet->type);
        if (type_code < Packet::TYPE_FIRST || type_code >= Packet::TYPE_LAST)
        {
            LOG_DEBUG << "unknown packet type, packet type = " << static_cast<unsigned>(type_code);
            return;
        }

        const size_t minimum_size = Packet::getSize(basic_packet->type);
        if (received_bytes < minimum_size)
        {
            LOG_DEBUG << "received_bytes < " << minimum_size;
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
            LOG_DEBUG << "unknown packet type, packet type = " << static_cast<unsigned>(type_code);
        }

        if (handler_return_value)
        {
            doReceive();
        }
    }

    void onGuaranteedAnswer(std::uint32_t packet_number)
    {
        GuaranteedDeliveryMap::iterator fit = guaranteed_delivery_map.find(packet_number);
        if (fit != guaranteed_delivery_map.end())
        {
            LOG_DEBUG << "found guaranteed request #" << packet_number;

            fit->second->delivered_ok = true;
            fit->second->timer.cancel();
            packet_pool.chainRemove(fit->second->packet_buffer_index);
            guaranteed_delivery_map.erase(fit);
        }
        else
        {
            LOG_WARNING << "guaranteed request is not found #" << packet_number;
        }
    }

    void onSend(void* packet_buffer, const boost::system::error_code& error, size_t transferred_bytes)
    {
        Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(packet_buffer);
        LOG_DEBUG << "onSend #" << basic_packet->packet_number;

        packet_pool.chainRemove(basic_packet->buffer_index);
    }

    void onGuaranteedSend(void* packet_buffer, const boost::system::error_code& error, size_t transferred_bytes)
    {
        Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(packet_buffer);
        LOG_DEBUG << "onGuaranteedSend #" << basic_packet->packet_number;
    }

    void onTimeout(GuaranteedDeliveryInfo::Ptr info, const boost::system::error_code& error)
    {
        if (info->delivered_ok)
        {
            Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(info->packet_buffer);
            LOG_INFO << "guaranteed delivered OK #" << info->packet_number;

            info->timer.cancel();
            guaranteed_delivery_map.erase(info->packet_number);
            return;
        }
        if (info->attempt_count)
        {
            Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(info->packet_buffer);
            LOG_WARNING << "guaranteed failed to delivery, new attempt #" << basic_packet->packet_number;

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
            Packet::Base* basic_packet = reinterpret_cast<Packet::Base*>(info->packet_buffer);
            LOG_ERROR << "guaranteeed failed to devilery #" << basic_packet->packet_number;

            info->timer.cancel();
            if (info->error_handler)
            {
                info->error_handler(info->packet_buffer, info->packet_buffer_size);
            }
            packet_pool.chainRemove(info->packet_buffer_index);
            guaranteed_delivery_map.erase(info->packet_number);
        }
    }
};

inline void onQuitSignal(const boost::system::error_code& error, int sig_number)
{
    Logger::onQuit();
    exit(EXIT_SUCCESS);
}
