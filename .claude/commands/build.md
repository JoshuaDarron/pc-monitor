# Build Command

Build the C++ backend and install Electron dependencies.

---

## Workflow

### 1. Install Electron Dependencies

```bash
pnpm install
```

### 2. Build C++ Backend

Configure and build with CMake using Visual Studio 2022:

```bash
mkdir -p build && cd build && cmake .. -G "Visual Studio 17 2022" -A x64 && cmake --build . --config Release
```

### 3. Verify Build Output

Confirm the backend executable was produced:

```bash
ls -la build/bin/Release/pc_monitor.exe
```

---

## Troubleshooting

- **CMake not found** — Ensure CMake is on PATH or installed via Visual Studio.
- **NVML not found** — GPU monitoring will fall back to estimates. Install CUDA Toolkit for full NVML support.
- **pnpm not found** — Install pnpm: `npm install -g pnpm`.
- **Build errors** — Check that Visual Studio 2022 Build Tools with C++ workload are installed.
