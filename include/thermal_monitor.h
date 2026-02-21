#pragma once

#include "metrics_types.h"
#include <vector>
#include <string>

namespace PCMonitor {

    class ThermalMonitor {
    private:
        bool wmi_initialized_;
        void* wmi_service_;

        struct SensorInfo {
            std::string type;
            std::string name;
            float offset;
        };

        std::vector<SensorInfo> system_sensors_;

        bool InitializeWMI();
        bool EnumerateSensors();
        std::vector<uint32_t> ReadFanSpeeds();

    public:
        ThermalMonitor();
        ~ThermalMonitor();

        bool Initialize();
        ThermalMetrics CollectMetrics();
        void Shutdown();
    };

}
