# Test Command

Verify the project builds and runs correctly.

---

## Context

This project does not have a formal test suite. Testing consists of build verification and runtime smoke checks.

---

## Workflow

### 1. Build Verification

Ensure the C++ backend compiles without errors:

```bash
cd build && cmake --build . --config Release 2>&1
```

Check for a successful build (exit code 0 and executable exists):

```bash
ls build/bin/Release/pc_monitor.exe
```

### 2. Backend Smoke Test

Start the backend and verify the API responds:

```bash
build/bin/Release/pc_monitor.exe &
sleep 2
curl -s http://localhost:8080/api/metrics | head -c 200
```

Then stop the backend process.

### 3. Report Results

Summarize what passed and what failed:
- **Build**: Did the C++ backend compile successfully?
- **API**: Did `/api/metrics` return valid JSON?
- **Issues**: List any errors or warnings encountered.
