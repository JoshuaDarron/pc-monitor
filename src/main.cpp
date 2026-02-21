#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Prevent old winsock.h from being included
#ifndef WINSOCK_API_LINKAGE
#define WINSOCK_API_LINKAGE
#endif

#include <winsock2.h>
#include <ws2tcpip.h>

// Now we can include other headers
#include "performance_monitor.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

// Global variables
PCMonitor::PerformanceMonitor* g_monitor = nullptr;
std::atomic<bool> g_web_server_running(false);

// Signal handler for graceful shutdown (Ctrl+C)
void SignalHandler(int signal) {
    std::cout << "\n\nReceived interrupt signal. Shutting down gracefully..." << std::endl;
    g_web_server_running = false;
    if (g_monitor) {
        g_monitor->Stop();
    }
    WSACleanup();
    exit(0);
}

// Helper to format a double with 1 decimal place without ostringstream
static std::string to_fixed1(double val) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f", val);
    return buf;
}

// Generate JSON response with current metrics
std::string GenerateJsonResponse(const PCMonitor::PerformanceMonitor& monitor) {
    const auto& gpu = monitor.GetGPUMetrics();
    const auto& cpu = monitor.GetCPUMetrics();
    const auto& ram = monitor.GetRAMMetrics();
    const auto& storage = monitor.GetStorageMetrics();
    const auto& power = monitor.GetPowerMetrics();
    const auto& thermal = monitor.GetThermalMetrics();

    std::string json;
    json.reserve(2048);

    json += "{\n";
    json += "  \"timestamp\": " + std::to_string(std::time(nullptr)) + ",\n";
    json += "  \"gpu\": {\n";
    json += "    \"vram_used_mb\": " + std::to_string(gpu.vram_used_mb) + ",\n";
    json += "    \"vram_total_mb\": " + std::to_string(gpu.vram_total_mb) + ",\n";
    json += "    \"core_clock_mhz\": " + std::to_string(gpu.core_clock_mhz) + ",\n";
    json += "    \"temperature_c\": " + std::to_string(gpu.temperature_c) + ",\n";
    json += "    \"utilization_percent\": " + std::to_string(gpu.utilization_percent) + ",\n";
    json += "    \"power_draw_w\": " + std::to_string(gpu.power_draw_w) + "\n";
    json += "  },\n";
    json += "  \"cpu\": {\n";
    json += "    \"utilization_percent\": " + to_fixed1(cpu.utilization_percent) + ",\n";
    json += "    \"temperature_c\": " + std::to_string(cpu.temperature_c) + ",\n";
    json += "    \"current_clock_mhz\": " + std::to_string(cpu.current_clock_mhz) + ",\n";
    json += "    \"core_count\": " + std::to_string(cpu.core_count) + ",\n";
    json += "    \"thread_count\": " + std::to_string(cpu.thread_count) + "\n";
    json += "  },\n";
    json += "  \"ram\": {\n";
    json += "    \"used_mb\": " + std::to_string(ram.used_mb) + ",\n";
    json += "    \"total_mb\": " + std::to_string(ram.total_mb) + ",\n";
    json += "    \"utilization_percent\": " + to_fixed1(ram.utilization_percent) + ",\n";
    json += "    \"speed_mhz\": " + std::to_string(ram.speed_mhz) + "\n";
    json += "  },\n";
    json += "  \"storage\": {\n";
    json += "    \"seq_read_mbps\": " + std::to_string(storage.seq_read_mbps) + ",\n";
    json += "    \"seq_write_mbps\": " + std::to_string(storage.seq_write_mbps) + ",\n";
    json += "    \"random_read_iops\": " + std::to_string(storage.random_read_iops) + ",\n";
    json += "    \"random_write_iops\": " + std::to_string(storage.random_write_iops) + "\n";
    json += "  },\n";
    json += "  \"power\": {\n";
    json += "    \"system_power_w\": " + std::to_string(power.system_power_w) + ",\n";
    json += "    \"cpu_power_w\": " + std::to_string(power.cpu_power_w) + ",\n";
    json += "    \"gpu_power_w\": " + std::to_string(power.gpu_power_w) + ",\n";
    json += "    \"psu_wattage\": " + std::to_string(power.psu_wattage) + ",\n";
    json += "    \"efficiency_percent\": " + to_fixed1(power.efficiency_percent) + "\n";
    json += "  },\n";
    json += "  \"thermal\": {\n";
    json += "    \"cpu_temp_c\": " + std::to_string(thermal.cpu_temp_c) + ",\n";
    json += "    \"gpu_temp_c\": " + std::to_string(thermal.gpu_temp_c) + ",\n";
    json += "    \"case_temp_c\": " + std::to_string(thermal.case_temp_c) + ",\n";
    json += "    \"fan_speeds_rpm\": [";
    for (size_t i = 0; i < thermal.fan_speeds_rpm.size(); ++i) {
        json += std::to_string(thermal.fan_speeds_rpm[i]);
        if (i < thermal.fan_speeds_rpm.size() - 1) json += ",";
    }
    json += "]\n";
    json += "  }\n";
    json += "}";

    return json;
}

// Read HTML file
std::string ReadHTMLFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return R"(<!DOCTYPE html>
<html>
<head>
    <title>PC Monitor</title>
    <style>
        body { font-family: Arial, sans-serif; background: #1a1a2e; color: white; margin: 40px; }
        .card { background: rgba(255,255,255,0.1); padding: 20px; margin: 20px 0; border-radius: 10px; }
        .metric { display: flex; justify-content: space-between; margin: 10px 0; }
        .value { font-weight: bold; color: #00ff88; }
        h1 { text-align: center; color: #00d4ff; }
        .api-link { color: #ffeb3b; text-decoration: none; }
        .error { color: #ff6b6b; }
    </style>
    <script>
        async function updateMetrics() {
            try {
                const response = await fetch('/api/metrics');
                const data = await response.json();
                document.getElementById('content').innerHTML = `
                    <div class="card">
                        <h2>🎮 GPU</h2>
                        <div class="metric"><span>VRAM:</span><span class="value">${data.gpu.vram_used_mb}/${data.gpu.vram_total_mb} MB</span></div>
                        <div class="metric"><span>Clock:</span><span class="value">${data.gpu.core_clock_mhz} MHz</span></div>
                        <div class="metric"><span>Temperature:</span><span class="value">${data.gpu.temperature_c}°C</span></div>
                        <div class="metric"><span>Usage:</span><span class="value">${data.gpu.utilization_percent}%</span></div>
                    </div>
                    <div class="card">
                        <h2>🔧 CPU</h2>
                        <div class="metric"><span>Cores/Threads:</span><span class="value">${data.cpu.core_count}/${data.cpu.thread_count}</span></div>
                        <div class="metric"><span>Usage:</span><span class="value">${data.cpu.utilization_percent}%</span></div>
                        <div class="metric"><span>Clock:</span><span class="value">${data.cpu.current_clock_mhz} MHz</span></div>
                        <div class="metric"><span>Temperature:</span><span class="value">${data.cpu.temperature_c}°C</span></div>
                    </div>
                    <div class="card">
                        <h2>💾 Memory</h2>
                        <div class="metric"><span>Usage:</span><span class="value">${data.ram.used_mb}/${data.ram.total_mb} MB (${data.ram.utilization_percent}%)</span></div>
                        <div class="metric"><span>Speed:</span><span class="value">DDR-${data.ram.speed_mhz}</span></div>
                    </div>
                    <div class="card">
                        <h2>⚡ Power</h2>
                        <div class="metric"><span>System:</span><span class="value">${data.power.system_power_w}W / ${data.power.psu_wattage}W</span></div>
                        <div class="metric"><span>Efficiency:</span><span class="value">${data.power.efficiency_percent}%</span></div>
                    </div>
                `;
            } catch (error) {
                document.getElementById('content').innerHTML = '<div class="error">Error loading data: ' + error.message + '</div>';
            }
        }
        
        setInterval(updateMetrics, 1000);
        window.onload = updateMetrics;
    </script>
</head>
<body>
    <h1>PC Performance Monitor</h1>
    <p>Dashboard file not found at web/dashboard.html. Using built-in simple dashboard.</p>
    <p>API: <a href="/api/metrics" class="api-link">/api/metrics</a></p>
    <div id="content">Loading...</div>
</body>
</html>)";
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

// Create HTTP response
std::string CreateHTTPResponse(const std::string& content, const std::string& content_type = "text/html") {
    std::string response;
    response.reserve(256 + content.length());
    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + content_type + "\r\n";
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Cache-Control: no-cache\r\n";
    response += "\r\n";
    response += content;
    return response;
}

// Handle HTTP request
std::string HandleRequest(const std::string& request, const PCMonitor::PerformanceMonitor& monitor) {
    if (request.find("GET /api/metrics") != std::string::npos) {
        std::string json = GenerateJsonResponse(monitor);
        return CreateHTTPResponse(json, "application/json");
    }
    else if (request.find("GET / ") != std::string::npos || request.find("GET /index.html") != std::string::npos) {
        std::string html = ReadHTMLFile("web/dashboard.html");
        return CreateHTTPResponse(html, "text/html");
    }
    else {
        std::string notFound = "<html><body><h1>404 Not Found</h1><p>Available endpoints:</p><ul><li><a href=\"/\">/</a> - Dashboard</li><li><a href=\"/api/metrics\">/api/metrics</a> - JSON API</li></ul></body></html>";
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(notFound.length()) + "\r\n\r\n" + notFound;
    }
}

// Web server thread function
void WebServerLoop(const PCMonitor::PerformanceMonitor& monitor, int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "❌ WSAStartup failed" << std::endl;
        return;
    }
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "❌ Socket creation failed" << std::endl;
        WSACleanup();
        return;
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(static_cast<u_short>(port));
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "❌ Bind failed on port " << port << ". Error: " << WSAGetLastError() << std::endl;
        std::cerr << "   Try a different port or check if another application is using port " << port << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
    
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "❌ Listen failed" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return;
    }
    
    std::cout << "🌐 Web server started successfully!" << std::endl;
    std::cout << "🔗 Dashboard: http://localhost:" << port << std::endl;
    std::cout << "📊 API: http://localhost:" << port << "/api/metrics" << std::endl;
    std::cout << std::endl;
    
    while (g_web_server_running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        
        timeval timeout = {};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int activity = select(0, &readfds, nullptr, nullptr, &timeout);
        
        if (activity > 0 && FD_ISSET(serverSocket, &readfds)) {
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET) {
                char buffer[4096];
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesReceived > 0) {
                    buffer[bytesReceived] = '\0';
                    std::string request(buffer);
                    std::string response = HandleRequest(request, monitor);
                    send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
                }
                
                closesocket(clientSocket);
            }
        }
    }
    
    closesocket(serverSocket);
    WSACleanup();
}

// Display usage information
void ShowUsage(const char* program_name) {
    std::cout << "PC Performance Monitor v1.0\n\n";
    std::cout << "Usage: " << program_name << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -w, --web         Enable web server mode\n";
    std::cout << "  -p, --port <num>  Web server port (default: 8080)\n";
    std::cout << "  -i, --interactive Interactive console mode (default if no -w)\n";
    std::cout << "  -h, --help        Show this help\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << "              # Interactive console mode\n";
    std::cout << "  " << program_name << " -w           # Web server on port 8080\n";
    std::cout << "  " << program_name << " -w -p 9000   # Web server on port 9000\n";
}

// Main function
int main(int argc, char* argv[]) {
    bool enable_web_server = false;
    int web_port = 8080;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--web" || arg == "-w") {
            enable_web_server = true;
        }
        else if (arg == "--port" || arg == "-p") {
            if (i + 1 < argc) {
                web_port = std::atoi(argv[++i]);
            }
        }
        else if (arg == "--help" || arg == "-h") {
            ShowUsage(argv[0]);
            return 0;
        }
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << "    PC Performance Monitor v1.0        " << std::endl;
    std::cout << "        High-Performance Edition        " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    // Create monitor instance
    PCMonitor::PerformanceMonitor monitor(std::chrono::milliseconds(1000));
    g_monitor = &monitor;
    
    // Set up signal handler
    signal(SIGINT, SignalHandler);
    
    std::cout << "🔧 Initializing performance monitor..." << std::endl;
    
    if (!monitor.Initialize()) {
        std::cerr << "❌ Failed to initialize performance monitor!" << std::endl;
        std::cerr << "   - Ensure you're running as Administrator" << std::endl;
        std::cerr << "   - Check that NVIDIA drivers are installed (for GPU monitoring)" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Monitor initialized successfully." << std::endl;
    
    if (!monitor.Start()) {
        std::cerr << "❌ Failed to start monitoring!" << std::endl;
        return 1;
    }
    
    std::cout << "🚀 Monitoring started successfully!" << std::endl;
    
    if (enable_web_server) {
        std::cout << "\n🌐 Starting web server mode..." << std::endl;
        g_web_server_running = true;
        
        std::thread webThread(WebServerLoop, std::ref(monitor), web_port);
        
        std::cout << "Web server is running. Press Ctrl+C to stop." << std::endl;
        std::cout << "Open your browser and navigate to the dashboard URL above!" << std::endl;
        
        // Keep main thread alive
        while (g_web_server_running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        if (webThread.joinable()) {
            webThread.join();
        }
    }
    else {
        std::cout << "\n📊 Running in console mode. Use --web to enable web interface." << std::endl;
        std::cout << "Press Ctrl+C to stop." << std::endl;
        
        // Simple console output every 5 seconds
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            std::cout << "📊 " << std::put_time(std::localtime(&time_t), "%H:%M:%S") 
                      << " - Monitoring active (data logged to pc_monitor_log.csv)" << std::endl;
        }
    }
    
    std::cout << "\n🛑 Stopping monitor..." << std::endl;
    monitor.Stop();
    std::cout << "✅ Monitor stopped successfully." << std::endl;
    
    return 0;
}
