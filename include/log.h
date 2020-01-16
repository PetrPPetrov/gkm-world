// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <boost/iostreams/stream.hpp>
#include <boost/system/error_code.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/iostreams/device/null.hpp>
#include "global_parameters.h"
#include "protocol_enum.h"

namespace Log
{
    struct Logger;
}

extern Log::Logger* g_logger;

namespace Log
{
    struct Message
    {
        Packet::ESeverityType severity = Packet::ESeverityType::InfoMessage;
        std::string text;
    };

    struct Logger
    {
        std::list<Message> messages;

        Logger(Packet::ESeverityType minimum_level_, const std::string& log_file_name_, bool log_to_screen_, bool log_to_file_)
            : log_file(log_file_name_, std::ios::app)
        {
            minimum_level = minimum_level_;
            log_file_name = log_file_name_;
            log_to_screen = log_to_screen_;
            log_to_file = log_to_file_;
        }

        struct Dumper
        {
            Dumper(Logger& logger_, Packet::ESeverityType severity_, bool ignore_) : logger(logger_), severity(severity_), ignore(ignore_)
            {
            }
            Dumper(Dumper&& other_dumper) noexcept : logger(other_dumper.logger), ignore(other_dumper.ignore), content(std::move(other_dumper.content))
            {
            }
            ~Dumper()
            {
                if (!ignore)
                {
                    std::string message = content.str();
                    const std::string severity_prefix = "[" + getText(severity) + "] ";
                    if (logger.log_to_screen)
                    {
                        std::cout << severity_prefix << message << std::endl;
                    }
                    if (logger.log_to_file)
                    {
                        logger.log_file << severity_prefix << message << std::endl;
                    }
                    if (logger.messages.size() >= MAX_LOG_MESSAGE_COUNT)
                    {
                        logger.messages.pop_front();
                    }
                    logger.messages.push_back({severity, message});
                }
            }
            std::ostream& getLog()
            {
                return content;
            }
        private:
            Logger& logger;
            Packet::ESeverityType severity = Packet::ESeverityType::InfoMessage;
            bool ignore = false;
            std::stringstream content;
        };

        Dumper getDumper(Packet::ESeverityType message_severity)
        {
            return Dumper(*this, message_severity, message_severity < minimum_level);
        }

    private:
        Packet::ESeverityType minimum_level;
        std::string log_file_name;
        bool log_to_screen;
        bool log_to_file;
        std::ofstream log_file;
    };

    #define LINE_SEPARATOR "********************************"
    #define ENDL_SEPARATOR "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"

    #define REPORT_TIME "[" << boost::posix_time::second_clock::local_time() << "] "
    #define REPORT_FILE_LINE "[" << __FILE__ << ":" << __LINE__ << "] "

    #define REAL_LOG(severity_type) g_logger->getDumper(severity_type).getLog()

    #if defined(_DEBUG)
    #define LOG_DEBUG REAL_LOG(Packet::ESeverityType::DebugMessage) << REPORT_TIME << REPORT_FILE_LINE
    #else
    static boost::iostreams::stream<boost::iostreams::null_sink> g_null_out(boost::iostreams::null_sink{});
    #define LOG_DEBUG g_null_out
    #endif

    #define LOG_INFO REAL_LOG(Packet::ESeverityType::InfoMessage) << REPORT_TIME << REPORT_FILE_LINE
    #define LOG_WARNING REAL_LOG(Packet::ESeverityType::WarningMessage) << REPORT_TIME << REPORT_FILE_LINE
    #define LOG_ERROR REAL_LOG(Packet::ESeverityType::ErrorMessage) << REPORT_TIME << REPORT_FILE_LINE
    #define LOG_FATAL REAL_LOG(Packet::ESeverityType::FatalMessage) << REPORT_TIME << REPORT_FILE_LINE

    inline void onQuitImpl()
    {
        if (g_logger)
        {
            LOG_INFO << "Server is shutting down...";
            delete g_logger;
            g_logger = nullptr;
        }
    }

    struct Holder
    {
        ~Holder()
        {
            onQuitImpl();
        }
    };

    inline void onQuit(const boost::system::error_code& error, int sig_number)
    {
        onQuitImpl();
        exit(EXIT_SUCCESS);
    }
}
