#pragma once

#include "metrics_types.h"
#include <string>

namespace PCMonitor {

    class PowerMonitor {
    private:
        bool hardware_support_;
        uint32_t psu_rated_wattage_;
        std::string psu_efficiency_rating_;

        bool DetectPowerHardware();
        uint32_t EstimateCPUPower(double utilization, uint32_t frequency);
        uint32_t EstimateGPUPower(uint32_t utilization, uint32_t frequency);
        double CalculateEfficiency(uint32_t actual_power);
        uint32_t ReadSystemPower();

    public:
        PowerMonitor();
        ~PowerMonitor();

        bool Initialize();
        PowerMetrics CollectMetrics(const CPUMetrics& cpu, const GPUMetrics& gpu);
        void Shutdown();

        void SetPSUSpecs(uint32_t wattage, const std::string& efficiency);
    };

}
