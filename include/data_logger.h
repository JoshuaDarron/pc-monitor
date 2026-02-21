#pragma once

#include "metrics_types.h"
#include <fstream>
#include <string>

namespace PCMonitor {

    class DataLogger {
    private:
        std::ofstream log_file_;
        std::string log_path_;
        
    public:
        DataLogger(const std::string& log_path);
        ~DataLogger();
        
        bool Initialize();
        void LogMetrics(const SystemMetrics& metrics);
        void Shutdown();
    };

}
