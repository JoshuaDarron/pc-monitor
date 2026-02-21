#include "power_monitor.h"
#include <iostream>
#include <algorithm>

namespace PCMonitor {

    PowerMonitor::PowerMonitor()
        : hardware_support_(false)
        , psu_rated_wattage_(850)
        , psu_efficiency_rating_("80+ Gold")
    {
    }

    PowerMonitor::~PowerMonitor() {
        Shutdown();
    }

    bool PowerMonitor::Initialize() {
        hardware_support_ = DetectPowerHardware();
        
        if (!hardware_support_) {
            std::cout << "Hardware power monitoring not available. Using estimation." << std::endl;
        }
        
        return true; // Always succeed, fallback to estimation
    }

    bool PowerMonitor::DetectPowerHardware() {
        // Check for hardware power monitoring capabilities
        // This would typically involve checking for:
        // - PSU with digital monitoring (Corsair Link, EVGA Precision, etc.)
        // - Motherboard power monitoring
        // - External power meters
        
        // For now, assume no hardware support
        return false;
    }

    PowerMetrics PowerMonitor::CollectMetrics(const CPUMetrics& cpu, const GPUMetrics& gpu) {
        PowerMetrics metrics = {};
        
        metrics.psu_wattage = psu_rated_wattage_;
        
        if (hardware_support_) {
            metrics.system_power_w = ReadSystemPower();
        } else {
            // Estimate power consumption
            metrics.cpu_power_w = EstimateCPUPower(cpu.utilization_percent, cpu.current_clock_mhz);
            metrics.gpu_power_w = EstimateGPUPower(gpu.utilization_percent, gpu.core_clock_mhz);
            
            // Add estimated power for other components
            uint32_t motherboard_power = 25;  // Motherboard, RAM, storage
            uint32_t fans_power = 15;         // Case fans
            uint32_t misc_power = 20;         // USB devices, etc.
            
            metrics.system_power_w = metrics.cpu_power_w + metrics.gpu_power_w + 
                                   motherboard_power + fans_power + misc_power;
        }
        
        metrics.efficiency_percent = CalculateEfficiency(metrics.system_power_w);
        
        return metrics;
    }

    uint32_t PowerMonitor::EstimateCPUPower(double utilization, uint32_t frequency) {
        // Base power consumption (idle)
        uint32_t base_power = 25;
        
        // TDP-based estimation (assuming 125W TDP for high-end CPU)
        uint32_t tdp = 125;
        
        // Power scales with utilization and frequency
        double load_factor = utilization / 100.0;
        double freq_factor = std::min(frequency / 3500.0, 1.5); // Normalize to base frequency
        
        uint32_t estimated_power = static_cast<uint32_t>(base_power + (tdp - base_power) * load_factor * freq_factor);
        
        return std::min(estimated_power, tdp + 20); // Cap at TDP + some overhead
    }

    uint32_t PowerMonitor::EstimateGPUPower(uint32_t utilization, uint32_t frequency) {
        // This would typically use actual GPU power draw from NVML
        // For estimation, assume the GPU power is already accurate from NVML
        
        // If NVML not available, estimate based on utilization
        uint32_t base_power = 30;  // Idle power
        uint32_t max_power = 350;  // Typical high-end GPU TDP
        
        double load_factor = utilization / 100.0;
        return static_cast<uint32_t>(base_power + (max_power - base_power) * load_factor);
    }

    double PowerMonitor::CalculateEfficiency(uint32_t actual_power) {
        // PSU efficiency curve (80+ Gold standard)
        double load_percentage = static_cast<double>(actual_power) / psu_rated_wattage_ * 100.0;
        
        double efficiency;
        if (load_percentage < 20) {
            efficiency = 82.0; // Low load efficiency
        } else if (load_percentage < 50) {
            efficiency = 85.0 + (load_percentage - 20) * 0.1; // Rising efficiency
        } else if (load_percentage < 80) {
            efficiency = 88.0; // Peak efficiency
        } else {
            efficiency = 88.0 - (load_percentage - 80) * 0.15; // Declining efficiency
        }
        
        return std::max(efficiency, 75.0); // Minimum efficiency
    }

    uint32_t PowerMonitor::ReadSystemPower() {
        // This would read from hardware power monitoring
        // Return 0 if not available
        return 0;
    }

    void PowerMonitor::SetPSUSpecs(uint32_t wattage, const std::string& efficiency) {
        psu_rated_wattage_ = wattage;
        psu_efficiency_rating_ = efficiency;
    }

    void PowerMonitor::Shutdown() {
        // Cleanup any hardware monitoring resources
    }

} 
