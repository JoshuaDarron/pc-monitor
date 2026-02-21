#pragma once

#include "metrics_types.h"

namespace PCMonitor {

    class ThermalMonitor {
    private:
        bool wmi_initialized_;
        
    public:
        ThermalMonitor();
        ~ThermalMonitor();
        
        bool Initialize();
        ThermalMetrics CollectMetrics();
        void Shutdown();
    };

}
