#pragma once

#include "metrics_types.h"
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>

// Only include NVML if available
#ifdef NVML_AVAILABLE
#include <nvml.h>
#endif

#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#pragma comment(lib, "pdh.lib")

#ifdef NVML_AVAILABLE
#pragma comment(lib, "nvml.lib")
#endif

namespace PCMonitor {

    class PerformanceMonitor {
    private:
        // Hardware monitoring handles
        PDH_HQUERY cpu_query_;
        PDH_HCOUNTER cpu_counter_;
        
        #ifdef NVML_AVAILABLE
        nvmlDevice_t gpu_device_;
        #else
        void* gpu_device_; // Placeholder when NVML not available
        #endif
        
        // Threading
        std::atomic<bool> running_;
        std::unique_ptr<std::thread> monitor_thread_;
        
        // Data collection
        std::chrono::milliseconds collection_interval_;
        std::ofstream log_file_;
        
        // Performance counters
        std::unordered_map<std::string, PDH_HCOUNTER> performance_counters_;
        
        // Metrics storage
        GPUMetrics gpu_metrics_;
        CPUMetrics cpu_metrics_;
        RAMMetrics ram_metrics_;
        StorageMetrics storage_metrics_;
        NetworkMetrics network_metrics_;
        PowerMetrics power_metrics_;
        ThermalMetrics thermal_metrics_;

        // Network session totals (raw byte counters from PDH)
        uint64_t total_bytes_received_;
        uint64_t total_bytes_sent_;

        // Cached CPU topology (never changes at runtime)
        uint32_t cached_core_count_;
        uint32_t cached_thread_count_;

        // Private methods
        bool InitializeNVML();
        bool InitializePDH();
        bool InitializeWMI();
        void CacheCPUTopology();
        
        void CollectGPUMetrics();
        void CollectCPUMetrics();
        void CollectRAMMetrics();
        void CollectStorageMetrics();
        void CollectNetworkMetrics();
        void CollectPowerMetrics();
        void CollectThermalMetrics();
        
        void LogMetrics();
        void MonitoringLoop();
        
    public:
        PerformanceMonitor(std::chrono::milliseconds interval = std::chrono::milliseconds(1000));
        ~PerformanceMonitor();
        
        bool Initialize();
        bool Start();
        void Stop();
        
        // Getters for current metrics
        const GPUMetrics& GetGPUMetrics() const { return gpu_metrics_; }
        const CPUMetrics& GetCPUMetrics() const { return cpu_metrics_; }
        const RAMMetrics& GetRAMMetrics() const { return ram_metrics_; }
        const StorageMetrics& GetStorageMetrics() const { return storage_metrics_; }
        const NetworkMetrics& GetNetworkMetrics() const { return network_metrics_; }
        const PowerMetrics& GetPowerMetrics() const { return power_metrics_; }
        const ThermalMetrics& GetThermalMetrics() const { return thermal_metrics_; }
        
        // Configuration
        void SetCollectionInterval(std::chrono::milliseconds interval);
        void SetLogFile(const std::string& filename);
    };

}
