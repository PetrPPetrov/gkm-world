// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <atomic>
#include <cassert>
#include <unordered_map>
#include <memory>
#include <fstream>
#include "gkm_world/fast_index.h"
#include "gkm_world/fixed_string.h"
#include "gkm_world/logger.h"

struct Message {
    Packet::ESeverityType severity = Packet::ESeverityType::InfoMessage;
    bool network_send = false;
    String512 text;
    std::atomic<bool> to_remove = false;
};

struct ThreadLogInfo {
    typedef std::shared_ptr<ThreadLogInfo> Ptr;

    ThreadLogInfo() = default;
    ThreadLogInfo(const ThreadLogInfo&) = delete;
    ThreadLogInfo(ThreadLogInfo&&) = delete;

    Memory::FastIndexRegistryChain<Message, 11> message_queue;
};

class InternalDumper {
    typedef std::unordered_map<std::thread::id, ThreadLogInfo::Ptr> ThreadInfo;
    ThreadInfo thread_info;
    ThreadInfo::const_iterator cur_thread_info_it;

    Packet::ESeverityType minimum_level = Packet::ESeverityType::DebugMessage;
    bool log_to_screen = false;
    bool log_to_file = false;
    std::string log_file_name;
    std::unique_ptr<std::ofstream> log_file;

    void dumpLocalMessage(const Message& message);
    void sendNetworkMessage(const Message& message);

public:
    InternalDumper() = default;
    InternalDumper(const InternalDumper&) = delete;
    InternalDumper(InternalDumper&&) = delete;

    void setLoggingToScreen(bool value);
    void setLoggingToFile(bool value, const std::string& file_name);
    void setMinimumLevel(Packet::ESeverityType level);
    void registerThread(std::thread::id thread_id);
    void registerThisThread();
    void reportMessage(Packet::ESeverityType severity, const std::string& text, bool network_send);
    void processNextQueuedMessage();
    void onQuit();
};

void InternalDumper::dumpLocalMessage(const Message& message) {
    const std::string severity_prefix = "[" + getText(message.severity) + "] ";
    if (log_to_screen) {
        std::cout << severity_prefix << message.text << std::endl;
    }
    if (log_to_file && log_file) {
        *log_file << severity_prefix << message.text << std::endl;
    }
}

void InternalDumper::sendNetworkMessage(const Message& message) {}

void InternalDumper::setLoggingToScreen(bool value) {
    log_to_screen = value;
}

void InternalDumper::setLoggingToFile(bool value, const std::string& file_name) {
    log_to_file = value;
    log_file_name = file_name;
    if (log_to_file) {
        log_file = std::make_unique<std::ofstream>(file_name, std::ios::app);
    } else {
        log_file = nullptr;
    }
}

void InternalDumper::registerThread(std::thread::id thread_id) {
    auto fit = thread_info.find(thread_id);
    assert(fit == thread_info.end());
    if (fit != thread_info.end()) {
        throw std::runtime_error((std::stringstream() << "InternalDumper: thread id " << thread_id << " is already registered").str());
    }

    auto new_thread_info = std::make_shared<ThreadLogInfo>();
    thread_info[thread_id] = new_thread_info;
    cur_thread_info_it = thread_info.begin();
}

void InternalDumper::registerThisThread() {
    registerThread(std::this_thread::get_id());
}

void InternalDumper::setMinimumLevel(Packet::ESeverityType level) {
    minimum_level = level;
}

void InternalDumper::reportMessage(Packet::ESeverityType severity, const std::string& text, bool network_send) {
    const std::thread::id thread_id = std::this_thread::get_id();
    auto fit = thread_info.find(thread_id);
    assert(fit != thread_info.end());
    if (fit == thread_info.end()) {
        throw std::runtime_error((std::stringstream() << "InternalDumper: unregistered thread id " << thread_id).str());
    }

    auto& message_queue = fit->second->message_queue;
    IndexType message_index;
    Message* new_message = message_queue.chainPushBack(message_index);
    new_message->severity = severity;
    new_message->text = text;
    new_message->network_send = network_send;
    new_message->to_remove = false;

    IndexType cur_index = message_queue.getChainHeadIndex();
    while (cur_index != INVALID_INDEX) {
        Message* cur_message = message_queue.get(cur_index);
        const IndexType next_index = message_queue.getChainNextIndex(cur_index);
        if (cur_message->to_remove) {
            message_queue.chainRemove(cur_index);
        }
        cur_index = next_index;
    }
}

void InternalDumper::processNextQueuedMessage() {
    if (thread_info.empty()) {
        return;
    }

    const auto& message_queue = cur_thread_info_it->second->message_queue;

    IndexType cur_index = message_queue.getChainHeadIndex();
    while (cur_index != INVALID_INDEX) {
        Message* cur_message = message_queue.get(cur_index);
        if (!cur_message->to_remove) {
            if (cur_message->network_send) {
                sendNetworkMessage(*cur_message);
            }
            dumpLocalMessage(*cur_message);
            cur_message->to_remove = true;
            break;
        }
        cur_index = message_queue.getChainNextIndex(cur_index);
    }

    ++cur_thread_info_it;
    if (cur_thread_info_it == thread_info.end()) {
        cur_thread_info_it = thread_info.begin();
    }
}

void InternalDumper::onQuit() {
    setLoggingToFile(false, "");
}

static InternalDumper g_logger;

void Logger::setLoggingToScreen(bool value) {
    g_logger.setLoggingToScreen(value);
}

void Logger::setLoggingToFile(bool value, const std::string& file_name) {
    g_logger.setLoggingToFile(value, file_name);
}

void Logger::setMinimumLevel(Packet::ESeverityType level) {
    g_logger.setMinimumLevel(level);
}

void Logger::registerThisThread() {
    g_logger.registerThisThread();
}

void Logger::reportMessage(Packet::ESeverityType severity, const std::string& text, bool network_send) {
    g_logger.reportMessage(severity, text, network_send);
}

void Logger::processNextQueuedMessage() {
    g_logger.processNextQueuedMessage();
}

void Logger::onQuit() {
    g_logger.onQuit();
}
