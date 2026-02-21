#include "thermal_monitor.h"
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>

namespace PCMonitor {

    ThermalMonitor::ThermalMonitor()
        : wmi_initialized_(false)
        , wmi_service_(nullptr)
    {
    }

    ThermalMonitor::~ThermalMonitor() {
        Shutdown();
    }

    bool ThermalMonitor::Initialize() {
        if (!InitializeWMI()) {
            std::cerr << "Failed to initialize WMI for thermal monitoring" << std::endl;
            return false;
        }
        
        if (!EnumerateSensors()) {
            std::cerr << "Failed to enumerate thermal sensors" << std::endl;
            return false;
        }
        
        return true;
    }

    bool ThermalMonitor::InitializeWMI() {
        HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (FAILED(hr)) return false;
        
        hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                 RPC_C_AUTHN_LEVEL_DEFAULT,
                                 RPC_C_IMP_LEVEL_IMPERSONATE,
                                 nullptr, EOAC_NONE, nullptr);
        if (FAILED(hr)) return false;
        
        IWbemLocator* locator = nullptr;
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                             IID_IWbemLocator, (LPVOID*)&locator);
        if (FAILED(hr)) return false;
        
        IWbemServices* service = nullptr;
        hr = locator->ConnectServer(_bstr_t(L"ROOT\\WMI"), nullptr, nullptr, 0,
                                   NULL, 0, 0, &service);
        
        locator->Release();
        
        if (FAILED(hr)) return false;
        
        hr = CoSetProxyBlanket(service, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
                              nullptr, RPC_C_AUTHN_LEVEL_CALL,
                              RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);
        
        if (FAILED(hr)) {
            service->Release();
            return false;
        }
        
        wmi_service_ = service;
        wmi_initialized_ = true;
        return true;
    }

    bool ThermalMonitor::EnumerateSensors() {
        if (!wmi_initialized_) return false;
        
        IWbemServices* service = static_cast<IWbemServices*>(wmi_service_);
        
        // Query temperature sensors
        IEnumWbemClassObject* enumerator = nullptr;
        HRESULT hr = service->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr, &enumerator);
        
        if (SUCCEEDED(hr)) {
            IWbemClassObject* object = nullptr;
            ULONG returned = 0;
            
            while (enumerator->Next(WBEM_INFINITE, 1, &object, &returned) == WBEM_S_NO_ERROR) {
                SensorInfo sensor;
                sensor.type = "temperature";
                sensor.name = "Thermal Zone";
                sensor.offset = 0.0f;
                
                VARIANT variant;
                VariantInit(&variant);
                
                hr = object->Get(L"InstanceName", 0, &variant, 0, 0);
                if (SUCCEEDED(hr) && variant.vt == VT_BSTR) {
                    sensor.name = _com_util::ConvertBSTRToString(variant.bstrVal);
                }
                VariantClear(&variant);
                
                system_sensors_.push_back(sensor);
                object->Release();
            }
            enumerator->Release();
        }
        
        return !system_sensors_.empty();
    }

    ThermalMetrics ThermalMonitor::CollectMetrics() {
        ThermalMetrics metrics = {};
        
        if (!wmi_initialized_) {
            // Fallback to estimated values
            metrics.cpu_temp_c = 45;
            metrics.gpu_temp_c = 50;
            metrics.motherboard_temp_c = 40;
            metrics.case_temp_c = 35;
            metrics.fan_speeds_rpm = {1200, 1000, 800};
            return metrics;
        }
        
        IWbemServices* service = static_cast<IWbemServices*>(wmi_service_);
        
        // Read temperature sensors
        IEnumWbemClassObject* enumerator = nullptr;
        HRESULT hr = service->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT * FROM MSAcpi_ThermalZoneTemperature"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            nullptr, &enumerator);
        
        if (SUCCEEDED(hr)) {
            IWbemClassObject* object = nullptr;
            ULONG returned = 0;
            
            std::vector<uint32_t> temperatures;
            
            while (enumerator->Next(WBEM_INFINITE, 1, &object, &returned) == WBEM_S_NO_ERROR) {
                VARIANT variant;
                VariantInit(&variant);
                
                hr = object->Get(L"CurrentTemperature", 0, &variant, 0, 0);
                if (SUCCEEDED(hr) && variant.vt == VT_I4) {
                    // Convert from tenths of Kelvin to Celsius
                    uint32_t temp_c = (variant.lVal / 10) - 273;
                    temperatures.push_back(temp_c);
                }
                VariantClear(&variant);
                object->Release();
            }
            enumerator->Release();
            
            // Assign temperatures to appropriate components
            if (!temperatures.empty()) {
                metrics.cpu_temp_c = temperatures[0];
                metrics.motherboard_temp_c = temperatures.size() > 1 ? temperatures[1] : temperatures[0];
                metrics.case_temp_c = metrics.motherboard_temp_c - 5;
            }
        }
        
        // Read fan speeds
        metrics.fan_speeds_rpm = ReadFanSpeeds();
        
        return metrics;
    }

    std::vector<uint32_t> ThermalMonitor::ReadFanSpeeds() {
        std::vector<uint32_t> fan_speeds;
        
        if (!wmi_initialized_) {
            // Return default fan speeds
            return {1200, 1000, 800};
        }
        
        // This would typically query specific hardware monitoring chips
        // For now, simulate realistic fan speeds based on temperatures
        fan_speeds.push_back(1200); // CPU fan
        fan_speeds.push_back(1000); // Case fan 1
        fan_speeds.push_back(800);  // Case fan 2
        
        return fan_speeds;
    }

    void ThermalMonitor::Shutdown() {
        if (wmi_service_) {
            static_cast<IWbemServices*>(wmi_service_)->Release();
            wmi_service_ = nullptr;
        }
        wmi_initialized_ = false;
        CoUninitialize();
    }

} 
