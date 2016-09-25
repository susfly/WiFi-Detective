const electron = require('electron');
// Module to control application life.
const {app} = electron;
// Module to create native browser window.
const {BrowserWindow} = electron;

const {ipcMain} = require('electron');

//const wlan = require("./build/Release/wlan");

//const notifier = require('node-notifier');
// String
//notifier.notify('Message');

// Object
/*notifier.notify({
'title': 'My notification',
'message': 'Hello, there!'
});*/

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let win;

function createWindow() {
    // Create the browser window.
    win = new BrowserWindow({
        width: 1080,
        height: 720,
        transparent: false,
        frame: false,
        backgroundColor: "#80FFFFFF",
        resizable: false
    });

    // and load the index.html of the app.
    win.loadURL(`file://${__dirname}/app/index.html`);

    // Open the DevTools.
    //win.webContents.openDevTools();


    // Emitted when the window is closed.
    win.on('closed', () => {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        win = null;
    });
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow);

// Quit when all windows are closed.
app.on('window-all-closed', () => {
    // On macOS it is common for applications and their menu bar
    // to stay active until the user quits explicitly with Cmd + Q
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', () => {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (win === null) {
        createWindow();
    }
});

ipcMain.on('close-main-window', function () {
    if (process.platform !== 'darwin') {
        //wlan.calculate_results_async(callme);
        app.quit()
    }
});
