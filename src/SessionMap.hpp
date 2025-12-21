#pragma once

#include <map>
#include <mutex>
#include <string>

#include "types.hpp"

class SessionMap {
    public:
        void add_session(const std::string& client_id, const SessionConfig& config) {
            std::lock_guard<std::mutex> lock(mutex_);
            sessions_[client_id] = config;
        }

        bool get_session(const std::string& client_id, SessionConfig& config) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = sessions_.find(client_id);
            if (it != sessions_.end()) {
                config = it->second;
                return true;
            }
            return false;
        }

        bool get_session_by_ip(const std::string& client_ip, SessionConfig& config) {
            std::lock_guard<std::mutex> lock(mutex_);
            for (const auto& pair : sessions_) {
                if (pair.second.client_address == client_ip) {
                    config = pair.second;
                    return true;
                }
            }
            return false;
        }

        void remove_session(const std::string& client_id) {
            std::lock_guard<std::mutex> lock(mutex_);
            sessions_.erase(client_id);
        }

    private:
        std::map<std::string, SessionConfig> sessions_;
        std::mutex mutex_;
};