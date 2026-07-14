const WebSocket = require('ws');

const ws = new WebSocket('wss://proxy.ftelnet.ca:2083/?host=telehack.com&port=23');

ws.on('open', () => {
    console.log('Connected!');
    ws.send('help\r\n');
});

ws.on('message', (data) => {
    console.log('Received:', data.toString());
    ws.close();
});

ws.on('error', (err) => {
    console.error('Error:', err.message);
});
