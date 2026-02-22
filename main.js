const { app, BrowserWindow } = require('electron');
const { spawn } = require('child_process');
const path = require('path');

const BACKEND_PORT = 8080;
const BACKEND_EXE = path.join(__dirname, 'build', 'bin', 'Release', 'pc_monitor.exe');

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
    titleBarOverlay: {
      color: '#00000000',
      symbolColor: '#b0b0b0',
      height: 36,
    },
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
