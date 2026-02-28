# PC Monitor - Claude Code Instructions

## Project Overview

Real-time PC hardware monitoring app with a C++ backend and Electron frontend dashboard.

## Architecture

- **Backend**: C++ (C++17) serving a REST API on port 8080
- **Frontend**: Electron app loading an HTML/CSS/JS dashboard (`web/dashboard.html`)
- **Entry points**: `main.js` (Electron), `src/main.cpp` (C++ backend)
- **API**: `/api/metrics` polled every second by the dashboard

## Key Directories

- `src/` - C++ source files (main, performance_monitor, data_logger, thermal_monitor, power_monitor, web_interface)
- `include/` - C++ headers
- `web/` - Dashboard HTML/CSS/JS (single-file `dashboard.html`)
- `assets/` - Icons and screenshots

## Build & Run

### C++ Backend (CMake)
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Electron App
```bash
pnpm install
pnpm start       # Dev mode (spawns C++ backend + Electron)
pnpm run dist    # Package distributable
```

## Platform

- **Windows only** (x64) - uses PDH, WMI, PSAPI, NVML
- Package manager: pnpm
- No test suite currently exists

## Conventions

- The dashboard is a single HTML file with inline CSS and JS
- Backend uses socket-based HTTP server (no external web framework)
- Metrics structs are defined in `include/metrics_types.h`
- Data logging outputs CSV format
- Recent UI work uses circular gauges and dynamic utilization-based color scales
