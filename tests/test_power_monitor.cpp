#include <gtest/gtest.h>
#include "power_monitor.h"

using namespace PCMonitor;

class PowerMonitorTest : public ::testing::Test {
protected:
    PowerMonitor monitor;
    CPUMetrics cpu{};
    GPUMetrics gpu{};

    void SetUp() override {
        monitor.Initialize();
        cpu.core_count = 8;
        cpu.thread_count = 16;
        cpu.base_clock_mhz = 3500;
        cpu.current_clock_mhz = 3500;
        gpu.core_clock_mhz = 1800;
    }
};

// --- EstimateCPUPower via CollectMetrics ---

TEST_F(PowerMonitorTest, CpuIdlePower) {
    cpu.utilization_percent = 0.0;
    cpu.current_clock_mhz = 3500;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // At 0% utilization, CPU power should be base_power (25W)
    EXPECT_EQ(metrics.cpu_power_w, 25u);
}

TEST_F(PowerMonitorTest, CpuFullLoadPower) {
    cpu.utilization_percent = 100.0;
    cpu.current_clock_mhz = 3500;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // At 100% utilization @ base frequency: base + (tdp - base) * 1.0 * 1.0 = 125W
    EXPECT_EQ(metrics.cpu_power_w, 125u);
}

TEST_F(PowerMonitorTest, CpuMidRangePower) {
    cpu.utilization_percent = 50.0;
    cpu.current_clock_mhz = 3500;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // 50% util @ base freq: 25 + (125-25)*0.5*1.0 = 75
    EXPECT_EQ(metrics.cpu_power_w, 75u);
}

TEST_F(PowerMonitorTest, CpuHighFrequencyBoost) {
    cpu.utilization_percent = 100.0;
    cpu.current_clock_mhz = 5250; // 5250/3500 = 1.5 (max freq_factor)
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // 100% util @ 1.5x freq: 25 + 100*1.0*1.5 = 175, capped at tdp+20 = 145
    EXPECT_EQ(metrics.cpu_power_w, 145u);
}

// --- EstimateGPUPower via CollectMetrics ---

TEST_F(PowerMonitorTest, GpuIdlePower) {
    gpu.utilization_percent = 0;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // At 0% utilization, GPU power should be base_power (30W)
    EXPECT_EQ(metrics.gpu_power_w, 30u);
}

TEST_F(PowerMonitorTest, GpuFullLoadPower) {
    gpu.utilization_percent = 100;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // At 100%: 30 + (350-30)*1.0 = 350
    EXPECT_EQ(metrics.gpu_power_w, 350u);
}

TEST_F(PowerMonitorTest, GpuMidRangePower) {
    gpu.utilization_percent = 50;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // 50%: 30 + 320*0.5 = 190
    EXPECT_EQ(metrics.gpu_power_w, 190u);
}

// --- CalculateEfficiency via CollectMetrics ---

TEST_F(PowerMonitorTest, EfficiencyLowLoad) {
    // Need system_power / psu_wattage < 20%
    // Default PSU = 850W, so need system_power < 170W
    cpu.utilization_percent = 0.0;
    cpu.current_clock_mhz = 3500;
    gpu.utilization_percent = 0;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // cpu=25 + gpu=30 + overhead=60 = 115W, 115/850 = 13.5% < 20%
    EXPECT_DOUBLE_EQ(metrics.efficiency_percent, 82.0);
}

TEST_F(PowerMonitorTest, EfficiencyMidLoad) {
    // Need 20-50% load: 170-425W for 850W PSU
    cpu.utilization_percent = 50.0;
    cpu.current_clock_mhz = 3500;
    gpu.utilization_percent = 30;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // cpu=75 + gpu=126 + overhead=60 = 261W
    // load% = 261/850*100 = 30.7%
    // efficiency = 85.0 + (30.7 - 20) * 0.1 = 86.07
    double expected_load = static_cast<double>(metrics.system_power_w) / 850.0 * 100.0;
    EXPECT_GE(expected_load, 20.0);
    EXPECT_LT(expected_load, 50.0);
    double expected_eff = 85.0 + (expected_load - 20.0) * 0.1;
    EXPECT_NEAR(metrics.efficiency_percent, expected_eff, 0.01);
}

TEST_F(PowerMonitorTest, EfficiencyPeakLoad) {
    // Need 50-80% load: 425-680W for 850W PSU
    cpu.utilization_percent = 100.0;
    cpu.current_clock_mhz = 5250;
    gpu.utilization_percent = 100;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    // cpu=145 + gpu=350 + overhead=60 = 555W
    // load% = 555/850*100 = 65.3%, in 50-80 range
    double load_pct = static_cast<double>(metrics.system_power_w) / 850.0 * 100.0;
    EXPECT_GE(load_pct, 50.0);
    EXPECT_LT(load_pct, 80.0);
    EXPECT_DOUBLE_EQ(metrics.efficiency_percent, 88.0);
}

// --- SetPSUSpecs ---

TEST_F(PowerMonitorTest, SetPSUSpecsAffectsEfficiency) {
    cpu.utilization_percent = 0.0;
    cpu.current_clock_mhz = 3500;
    gpu.utilization_percent = 0;

    // With default 850W PSU, 115W = 13.5% load -> 82% efficiency
    auto metrics850 = monitor.CollectMetrics(cpu, gpu);
    EXPECT_DOUBLE_EQ(metrics850.efficiency_percent, 82.0);

    // With 500W PSU, 115W = 23% load -> mid-load efficiency
    monitor.SetPSUSpecs(500, "80+ Gold");
    auto metrics500 = monitor.CollectMetrics(cpu, gpu);
    EXPECT_GT(metrics500.efficiency_percent, 82.0);
    EXPECT_EQ(metrics500.psu_wattage, 500u);
}

// --- CollectMetrics: system power = cpu + gpu + 60W overhead ---

TEST_F(PowerMonitorTest, SystemPowerIsSum) {
    cpu.utilization_percent = 50.0;
    cpu.current_clock_mhz = 3500;
    gpu.utilization_percent = 50;
    auto metrics = monitor.CollectMetrics(cpu, gpu);
    uint32_t overhead = 25 + 15 + 20; // motherboard + fans + misc = 60
    EXPECT_EQ(metrics.system_power_w, metrics.cpu_power_w + metrics.gpu_power_w + overhead);
}
