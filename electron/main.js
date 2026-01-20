const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const { spawn } = require('child_process');

let mainWindow;
let aiProcess;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 800,
    height: 900,
    webPreferences: {
      nodeIntegration: true,
      contextIsolation: false
    }
  });

  mainWindow.loadFile('index.html');

  // Spawn Universal AI process
  const aiPath = path.join(__dirname, '..', 'cpp', 'universal_ai');
  aiProcess = spawn(aiPath);

  aiProcess.stdout.on('data', (data) => {
    const rawOutput = data.toString().trim();
    console.log(`AI Output: ${rawOutput}`);
    // Handle multiple lines in one output
    const lines = rawOutput.split('\n');
    lines.forEach(line => {
      mainWindow.webContents.send('ai-response', line.trim());
    });
  });

  aiProcess.stderr.on('data', (data) => {
    console.error(`AI Error: ${data}`);
  });

  aiProcess.on('close', (code) => {
    console.log(`AI process exited with code ${code}`);
  });
}

ipcMain.on('send-to-ai', (event, command) => {
  if (aiProcess) {
    aiProcess.stdin.write(command + '\n');
  }
});

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
  if (aiProcess) aiProcess.kill();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});
