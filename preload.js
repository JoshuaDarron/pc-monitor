const { contextBridge, ipcRenderer, nativeTheme } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  platform: process.platform,
  isDarkTheme: nativeTheme.shouldUseDarkColors,
  onThemeChange: (callback) => {
    ipcRenderer.on('theme-changed', (_, isDark) => callback(isDark));
  },
});
