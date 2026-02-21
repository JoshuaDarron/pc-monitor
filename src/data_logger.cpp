#include "data_logger.h"
#include "performance_monitor.h"
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace PCMonitor {

    DataLogger::DataLogger(const std::string& log_path, size_t max_size_mb, bool rotate)
        : log_path_(log_path)
        , max_file_size_(max_size_mb * 1024 * 1024)
        , rotate_logs_(rotate)
        , logging_active_(false)
        , entries_logged_(0)
        , bytes_written_(0)
    {
    }

    DataLogger::~DataLogger() {
        Shutdown();
    }

    bool DataLogger::Initialize() {
        if (!OpenLogFile()) {
            return false;
        }
        
        WriteHeader();
        
        // Start async logging thread
        logging_active_ = true;
        logging_thread_ = std::make_unique<std::thread>(&DataLogger::LoggingLoop, this);
        
        return true;
    }

    bool DataLogger::OpenLogFile() {
        log_file_.open(log_path_, std::ios::app | std::ios::binary);
        return log_file_.is_open();
    }

    void DataLogger::WriteHeader() {
        if (log_file_.tellp() == 0) { // File is empty, write header
            log_file_ << "Timestamp,CPU_Usage_%,CPU_Temp_C,CPU_Clock_MHz,"
                      << "GPU_Usage_%,GPU_Temp_C,GPU_Clock_MHz,GPU_VRAM_Used_MB,"
                      << "RAM_Usage_%,RAM_Used_MB,"
                      << "Storage_Read_MBps,Storage_Write_MBps,"
                      << "System_Power_W,PSU_Efficiency_%,"
                      << "Case_Temp_C,Fan1_RPM,Fan2_RPM,Fan3_RPM\n";
        }
    }

    void DataLogger::LogMetrics(const SystemMetrics& metrics) {
        if (!logging_active_) return;
        
        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.metrics = metrics;
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            log_queue_.push(entry);
        }
        queue_cv_.notify_one();
    }

    void DataLogger::LoggingLoop() {
        while (logging_active_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !log_queue_.empty() || !logging_active_; });
            
            while (!log_queue_.empty() && logging_active_) {
                LogEntry entry = log_queue_.front();
                log_queue_.pop();
                lock.unlock();
                
                std::string log_line = FormatLogEntry(entry);
                log_file_ << log_line;
                log_file_.flush();
                
                bytes_written_ += log_line.length();
                entries_logged_++;
                
                // Check for log rotation
                if (rotate_logs_ && bytes_written_ > max_file_size_) {
                    RotateLogFile();
                    bytes_written_ = 0;
                }
                
                lock.lock();
            }
        }
    }

    std::string DataLogger::FormatLogEntry(const LogEntry& entry) {
        std::ostringstream oss;
        auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
        
        oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << ","
            << std::fixed << std::setprecision(2) << entry.metrics.ram.utilization_percent << ","
            << entry.metrics.ram.used_mb << ","
            << entry.metrics.storage.seq_read_mbps << ","
            << entry.metrics.storage.seq_write_mbps << ","
            << entry.metrics.power.system_power_w << ","
            << std::fixed << std::setprecision(2) << entry.metrics.power.efficiency_percent << ","
            << entry.metrics.thermal.case_temp_c << ",";
        
        // Fan speeds (up to 3 fans)
        for (size_t i = 0; i < 3; ++i) {
            if (i < entry.metrics.thermal.fan_speeds_rpm.size()) {
                oss << entry.metrics.thermal.fan_speeds_rpm[i];
            } else {
                oss << "0";
            }
            if (i < 2) oss << ",";
        }
        oss << "\n";
        
        return oss.str();
    }

    void DataLogger::RotateLogFile() {
        log_file_.close();
        
        // Create timestamped backup
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::ostringstream backup_name;
        backup_name << log_path_ << "." << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        
        try {
            std::filesystem::rename(log_path_, backup_name.str());
        } catch (const std::exception& e) {
            std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
        }
        
        OpenLogFile();
        WriteHeader();
    }

    void DataLogger::Shutdown() {
        logging_active_ = false;
        queue_cv_.notify_all();
        
        if (logging_thread_ && logging_thread_->joinable()) {
            logging_thread_->join();
        }
        
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

} 
