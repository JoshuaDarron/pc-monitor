#pragma once

#include <cstdint>
#include <vector>
#include <chrono>
#include <string>

namespace PCMonitor {

    struct GPUMetrics {
        uint32_t vram_total_mb;
        uint32_t vram_used_mb;
        uint32_t core_clock_mhz;
        uint32_t memory_clock_mhz;
        uint32_t temperature_c;
        uint32_t power_draw_w;
        uint32_t utilization_percent;
        uint64_t memory_bandwidth_mbps;
    };

    struct CPUMetrics {
        uint32_t core_count;
        uint32_t thread_count;
        uint32_t base_clock_mhz;
        uint32_t current_clock_mhz;
        uint32_t temperature_c;
        double utilization_percent;
        uint32_t l3_cache_mb;
    };

    struct RAMMetrics {
        uint64_t total_mb;
        uint64_t used_mb;
        uint32_t speed_mhz;
        uint32_t latency_cl;
        double utilization_percent;
    };

    struct StorageMetrics {
        uint64_t seq_read_mbps;
        uint64_t seq_write_mbps;
        uint64_t random_read_iops;
        uint64_t random_write_iops;
        uint32_t temperature_c;
        double health_percent;
    };

    struct PowerMetrics {
        uint32_t psu_wattage;
        uint32_t system_power_w;
        uint32_t cpu_power_w;
        uint32_t gpu_power_w;
        double efficiency_percent;
    };

    struct ThermalMetrics {
        uint32_t cpu_temp_c;
        uint32_t gpu_temp_c;
        uint32_t motherboard_temp_c;
        uint32_t case_temp_c;
        std::vector<uint32_t> fan_speeds_rpm;
    };

    struct SystemMetrics {
        GPUMetrics gpu;
        CPUMetrics cpu;
        RAMMetrics ram;
        StorageMetrics storage;
        PowerMetrics power;
        ThermalMetrics thermal;
    };

}
