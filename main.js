const { app, BrowserWindow, nativeTheme } = require('electron');
const { spawn } = require('child_process');
const path = require('path');

const BACKEND_PORT = 8080;
const isPackaged = app.isPackaged;
const BACKEND_EXE = isPackaged
  ? path.join(process.resourcesPath, 'backend', 'pc_monitor.exe')
  : path.join(__dirname, 'build', 'bin', 'Release', 'pc_monitor.exe');

let mainWindow = null;
let backendProcess = null;

function spawnBackend() {
  backendProcess = spawn(BACKEND_EXE, ['-w', '-p', String(BACKEND_PORT)], {
    stdio: 'ignore',
    windowsHide: true,
  });

  backendProcess.on('error', (err) => {
    console.error('Failed to start backend:', err.message);
  });

  backendProcess.on('exit', (code) => {
    console.log('Backend exited with code', code);
    backendProcess = null;
  });
}

function killBackend() {
  if (backendProcess) {
    backendProcess.kill();
    backendProcess = null;
  }
}

function getTitleBarOverlay() {
  const isDark = nativeTheme.shouldUseDarkColors;
  return {
    color: '#00000000',
    symbolColor: isDark ? '#b0b0b0' : '#666666',
    height: 36,
  };
}

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1200,
    height: 800,
    minWidth: 800,
    minHeight: 600,
    title: 'PC Monitor Pro',
    icon: path.join(__dirname, 'web', 'favicon.ico'),
    autoHideMenuBar: true,
    titleBarStyle: 'hidden',
    titleBarOverlay: getTitleBarOverlay(),
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  mainWindow.loadFile(path.join(__dirname, 'web', 'dashboard.html'));

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

nativeTheme.on('updated', () => {
  const isDark = nativeTheme.shouldUseDarkColors;
  if (mainWindow) {
    mainWindow.webContents.send('theme-changed', isDark);
    mainWindow.setTitleBarOverlay(getTitleBarOverlay());
  }
});

app.whenReady().then(() => {
  spawnBackend();
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  killBackend();
  app.quit();
});

app.on('before-quit', () => {
  killBackend();
});
