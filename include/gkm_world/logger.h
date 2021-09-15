// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <string>
#include <sstream>
#include <thread>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "gkm_world/gkm_world.h"
#include "gkm_world/protocol_enum.h"

namespace Logger {
    void setLoggingToScreen(bool value);
    void setLoggingToFile(bool value, const std::string& file_name);
    void setMinimumLevel(Packet::ESeverityType level);
    void registerThisThread();
    void reportMessage(Packet::ESeverityType severity, const std::string& text, bool network_send);
    void processNextQueuedMessage();
    void onQuit();

    struct Dumper {
        Dumper(Packet::ESeverityType severity_, bool network_send_) : severity(severity_), network_send(network_send_) {}
        ~Dumper() {
            reportMessage(severity, content.str(), network_send);
        }
        std::stringstream& getOutputStream() {
            return content;
        }
    private:
        Packet::ESeverityType severity = Packet::ESeverityType::InfoMessage;
        bool network_send = false;
        std::stringstream content;
    };

    struct DummyDumper {};

    template<class T>
    const DummyDumper& operator<<(const DummyDumper& dummy_dumper, const T&) {
        return dummy_dumper;
    }

#define LINE_SEPARATOR "********************************"
#define ENDL_SEPARATOR "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"

#define REPORT_TIME "[" << boost::posix_time::second_clock::local_time() << "] "
#define REPORT_FILE_LINE "[" << __FILE__ << ":" << __LINE__ << "] "

#define LOCAL_DUMPER(severity_type) Logger::Dumper(severity_type, false).getOutputStream()
#define NETWORK_DUMPER(severity_type) Logger::Dumper(severity_type, true).getOutputStream()

#if defined(DEBUG_LOG)
#define LOG_DEBUG LOCAL_DUMPER(Packet::ESeverityType::DebugMessage) << REPORT_TIME << REPORT_FILE_LINE
#define NET_LOG_DEBUG NETWORK_DUMPER(Packet::ESeverityType::DebugMessage) << REPORT_TIME << REPORT_FILE_LINE
#else
#define LOG_DEBUG Logger::DummyDumper()
#define NET_LOG_DEBUG Logger::DummyDumper()
#endif

#define LOG_INFO LOCAL_DUMPER(Packet::ESeverityType::InfoMessage) << REPORT_TIME << REPORT_FILE_LINE
#define LOG_WARNING LOCAL_DUMPER(Packet::ESeverityType::WarningMessage) << REPORT_TIME << REPORT_FILE_LINE
#define LOG_ERROR LOCAL_DUMPER(Packet::ESeverityType::ErrorMessage) << REPORT_TIME << REPORT_FILE_LINE
#define LOG_FATAL LOCAL_DUMPER(Packet::ESeverityType::FatalMessage) << REPORT_TIME << REPORT_FILE_LINE
#define NET_LOG_INFO NETWORK_DUMPER(Packet::ESeverityType::InfoMessage) << REPORT_TIME << REPORT_FILE_LINE
#define NET_LOG_WARNING NETWORK_DUMPER(Packet::ESeverityType::WarningMessage) << REPORT_TIME << REPORT_FILE_LINE
#define NET_LOG_ERROR NETWORK_DUMPER(Packet::ESeverityType::ErrorMessage) << REPORT_TIME << REPORT_FILE_LINE
#define NET_LOG_FATAL NETWORK_DUMPER(Packet::ESeverityType::FatalMessage) << REPORT_TIME << REPORT_FILE_LINE
}
