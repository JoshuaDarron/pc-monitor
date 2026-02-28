# Dev Command

Start the app in development mode.

---

## Prerequisites

Before running, ensure:
1. Electron dependencies are installed (`pnpm install`).
2. The C++ backend is built (`build/bin/Release/pc_monitor.exe` exists).

If either is missing, run `/build` first.

---

## Workflow

### 1. Check Prerequisites

```bash
ls build/bin/Release/pc_monitor.exe && ls node_modules/.pnpm
```

If either check fails, run the build steps before proceeding.

### 2. Start the App

```bash
pnpm start
```

This launches Electron, which spawns the C++ backend and loads the dashboard.

---

## Notes

- The C++ backend serves the REST API on port **8080**.
- The dashboard polls `/api/metrics` every second.
- The Electron app is defined in `main.js` and loads `web/dashboard.html`.
- To iterate on the dashboard UI, edit `web/dashboard.html` and reload the Electron window.
