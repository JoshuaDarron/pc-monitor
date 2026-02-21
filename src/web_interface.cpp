#include "web_interface.h"
#include "performance_monitor.h"
#include <sstream>
#include <iostream>
#include <thread>

namespace PCMonitor {

    WebInterface::WebInterface(uint16_t port)
        : server_running_(false)
        , port_(port)
        , monitor_(nullptr)
    {
    }

    WebInterface::~WebInterface() {
        Stop();
    }

    bool WebInterface::Start(PerformanceMonitor* monitor) {
        if (server_running_ || !monitor) {
            return false;
        }
        
        monitor_ = monitor;
        server_running_ = true;
        server_thread_ = std::make_unique<std::thread>(&WebInterface::ServerLoop, this);
        
        std::cout << "Web interface started on " << GetUrl() << std::endl;
        return true;
    }

    void WebInterface::Stop() {
        if (!server_running_) return;
        
        server_running_ = false;
        
        if (server_thread_ && server_thread_->joinable()) {
            server_thread_->join();
        }
        
        server_thread_.reset();
        monitor_ = nullptr;
    }

    void WebInterface::ServerLoop() {
        // This is a simplified HTTP server implementation
        // In a real application, you'd use a proper HTTP library like cpp-httplib
        
        while (server_running_) {
            // Simulate serving HTTP requests
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // In a real implementation, this would:
            // 1. Listen on socket
            // 2. Accept connections
            // 3. Parse HTTP requests
            // 4. Generate responses
            // 5. Send responses back to clients
        }
    }

    std::string WebInterface::GenerateJsonResponse() const {
        if (!monitor_) return "{}";
        
        auto gpu = monitor_->GetGPUMetrics();
        auto cpu = monitor_->GetCPUMetrics();
        auto ram = monitor_->GetRAMMetrics();
        auto storage = monitor_->GetStorageMetrics();
        auto power = monitor_->GetPowerMetrics();
        auto thermal = monitor_->GetThermalMetrics();
        
        std::ostringstream json;
        json << "{\n";
        json << "  \"gpu\": {\n";
        json << "    \"vram_used_mb\": " << gpu.vram_used_mb << ",\n";
        json << "    \"vram_total_mb\": " << gpu.vram_total_mb << ",\n";
        json << "    \"core_clock_mhz\": " << gpu.core_clock_mhz << ",\n";
        json << "    \"temperature_c\": " << gpu.temperature_c << ",\n";
        json << "    \"utilization_percent\": " << gpu.utilization_percent << "\n";
        json << "  },\n";
        json << "  \"cpu\": {\n";
        json << "    \"utilization_percent\": " << cpu.utilization_percent << ",\n";
        json << "    \"temperature_c\": " << cpu.temperature_c << ",\n";
        json << "    \"current_clock_mhz\": " << cpu.current_clock_mhz << ",\n";
        json << "    \"core_count\": " << cpu.core_count << "\n";
        json << "  },\n";
        json << "  \"ram\": {\n";
        json << "    \"used_mb\": " << ram.used_mb << ",\n";
        json << "    \"total_mb\": " << ram.total_mb << ",\n";
        json << "    \"utilization_percent\": " << ram.utilization_percent << "\n";
        json << "  },\n";
        json << "  \"power\": {\n";
        json << "    \"system_power_w\": " << power.system_power_w << ",\n";
        json << "    \"psu_wattage\": " << power.psu_wattage << ",\n";
        json << "    \"efficiency_percent\": " << power.efficiency_percent << "\n";
        json << "  }\n";
        json << "}";
        
        return json.str();
    }

    std::string WebInterface::GetUrl() const {
        return "http://localhost:" + std::to_string(port_);
    }

} 
