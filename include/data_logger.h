#pragma once

#include "metrics_types.h"
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <chrono>
#include <memory>

namespace PCMonitor {

    class DataLogger {
    private:
        std::ofstream log_file_;
        std::string log_path_;
        size_t max_file_size_;
        bool rotate_logs_;
        std::atomic<bool> logging_active_;
        uint64_t entries_logged_;
        size_t bytes_written_;

        std::unique_ptr<std::thread> logging_thread_;
        std::mutex queue_mutex_;
        std::condition_variable queue_cv_;

        struct LogEntry {
            std::chrono::system_clock::time_point timestamp;
            SystemMetrics metrics;
        };

        std::queue<LogEntry> log_queue_;

        bool OpenLogFile();
        void WriteHeader();
        void LoggingLoop();
        std::string FormatLogEntry(const LogEntry& entry);
        void RotateLogFile();

    public:
        DataLogger(const std::string& log_path, size_t max_size_mb = 100, bool rotate = true);
        ~DataLogger();

        bool Initialize();
        void LogMetrics(const SystemMetrics& metrics);
        void Shutdown();
    };

}
