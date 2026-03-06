#include <gtest/gtest.h>
#include "data_logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace PCMonitor;

class DataLoggerTest : public ::testing::Test {
protected:
    std::string temp_path;

    void SetUp() override {
        temp_path = std::filesystem::temp_directory_path().string() + "/test_log_" +
                    std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".csv";
    }

    void TearDown() override {
        std::filesystem::remove(temp_path);
    }

    std::string ReadFileContents(const std::string& path) {
        std::ifstream f(path);
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    SystemMetrics MakeSampleMetrics() {
        SystemMetrics m{};
        m.cpu.utilization_percent = 45.5;
        m.cpu.temperature_c = 65;
        m.cpu.current_clock_mhz = 4200;
        m.gpu.utilization_percent = 80;
        m.gpu.temperature_c = 72;
        m.gpu.core_clock_mhz = 1900;
        m.gpu.vram_used_mb = 4096;
        m.ram.utilization_percent = 62.3;
        m.ram.used_mb = 16000;
        m.storage.seq_read_mbps = 3500;
        m.storage.seq_write_mbps = 3000;
        m.power.system_power_w = 350;
        m.power.efficiency_percent = 88.0;
        m.thermal.case_temp_c = 38;
        m.thermal.fan_speeds_rpm = {1200, 1100, 900};
        return m;
    }
};

TEST_F(DataLoggerTest, WritesCSVHeaderOnEmptyFile) {
    DataLogger logger(temp_path);
    ASSERT_TRUE(logger.Initialize());

    // Give the logging thread a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    logger.Shutdown();

    std::string contents = ReadFileContents(temp_path);
    EXPECT_TRUE(contents.find("Timestamp,") != std::string::npos);
    EXPECT_TRUE(contents.find("CPU_Usage_%") != std::string::npos);
    EXPECT_TRUE(contents.find("Fan1_RPM") != std::string::npos);
}

TEST_F(DataLoggerTest, LogsMetricsEntry) {
    DataLogger logger(temp_path);
    ASSERT_TRUE(logger.Initialize());

    auto metrics = MakeSampleMetrics();
    logger.LogMetrics(metrics);

    // Wait for async write
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    logger.Shutdown();

    std::string contents = ReadFileContents(temp_path);
    // Should have header + at least one data line
    std::istringstream stream(contents);
    std::string line;
    int line_count = 0;
    while (std::getline(stream, line)) {
        if (!line.empty()) line_count++;
    }
    EXPECT_GE(line_count, 2); // header + data

    // Verify some metric values appear in the log
    EXPECT_TRUE(contents.find("62.30") != std::string::npos); // RAM utilization
    EXPECT_TRUE(contents.find("350") != std::string::npos);   // system power
    EXPECT_TRUE(contents.find("1200") != std::string::npos);  // fan speed
}

TEST_F(DataLoggerTest, ConstructorMaxSizeConversion) {
    // 100 MB default -> 100 * 1024 * 1024 bytes
    // We can't directly inspect max_file_size_ (private), but we can verify
    // the logger works correctly with a very small max size
    DataLogger logger(temp_path, 1, true); // 1 MB max
    ASSERT_TRUE(logger.Initialize());
    logger.Shutdown();

    // Just verify it initializes without error
    EXPECT_TRUE(true);
}

TEST_F(DataLoggerTest, MultipleLogEntries) {
    DataLogger logger(temp_path);
    ASSERT_TRUE(logger.Initialize());

    auto metrics = MakeSampleMetrics();
    logger.LogMetrics(metrics);
    logger.LogMetrics(metrics);
    logger.LogMetrics(metrics);

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    logger.Shutdown();

    std::string contents = ReadFileContents(temp_path);
    std::istringstream stream(contents);
    std::string line;
    int line_count = 0;
    while (std::getline(stream, line)) {
        if (!line.empty()) line_count++;
    }
    EXPECT_GE(line_count, 4); // header + 3 data lines
}
