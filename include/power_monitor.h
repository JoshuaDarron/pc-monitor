#pragma once

#include "metrics_types.h"

namespace PCMonitor {

    class PowerMonitor {
    private:
        bool hardware_support_;
        uint32_t psu_rated_wattage_;
        
    public:
        PowerMonitor();
        ~PowerMonitor();
        
        bool Initialize();
        PowerMetrics CollectMetrics(const CPUMetrics& cpu, const GPUMetrics& gpu);
        void Shutdown();
        
        void SetPSUSpecs(uint32_t wattage, const std::string& efficiency);
    };

}
