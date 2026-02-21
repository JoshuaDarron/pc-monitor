#pragma once

#include "metrics_types.h"
#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace PCMonitor {

    // Forward declaration to avoid circular dependency
    class PerformanceMonitor;

    class WebInterface {
    private:
        std::atomic<bool> server_running_;
        std::unique_ptr<std::thread> server_thread_;
        uint16_t port_;
        PerformanceMonitor* monitor_;

        void ServerLoop();
        std::string GenerateJsonResponse() const;
        std::string GetUrl() const;

    public:
        WebInterface(uint16_t port = 8080);
        ~WebInterface();

        bool Start(PerformanceMonitor* monitor);
        void Stop();

        bool IsRunning() const { return server_running_; }
        uint16_t GetPort() const { return port_; }
    };

}
