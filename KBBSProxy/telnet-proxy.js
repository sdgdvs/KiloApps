const { WebSocketServer } = require('ws');
const { Socket } = require('net');
const http = require('http');

const PORT = process.env.PORT || 8080;

const server = http.createServer((req, res) => {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('KBBS WebSocket-to-Telnet Proxy is running.\n');
});

const wss = new WebSocketServer({ server });

wss.on('connection', (ws, req) => {
    const url = new URL(req.url, `http://${req.headers.host}`);
    const host = url.searchParams.get('host');
    const port = parseInt(url.searchParams.get('port') || '23', 10);

    const ALLOWED_HOSTS = [
        '20forbeers.com', 'mutinybbs.com', 'borderlinebbs.dyndns.org', 'din.asciiattic.com',
        'bbs.bottomlessabyss.net', 'bbs.darkrealms.ca', 'bbs.throwbackbbs.com', 'vert.synchro.net',
        'bbs.electronicchicken.com', 'capitolcityonline.net', 'bbs.agency', 'xibalba.l33t.codes',
        'blackflag.acid.org', 'bbs.absinthe.ws', 'bbs.thebrokenbubble.com', 'ttb.rgbbs.info',
        'bbs.alsgeeklab.com', 'bbs.retrocampus.com', 'bbs.airandwave.net', 'bbs.nz',
        'blocktronics.org', 'bbs.thekeep.net', '13leader.net', 'bbs.8bitboyz.com',
        'particlesbbs.dyndns.org', '300baud.dynu.net', 'bbs.fozztexx.com', 'clutchbbs.com',
        'bbs.endofthelinebbs.com'
    ];

    if (!host) {
        console.error('Connection rejected: Missing host parameter');
        ws.close(1008, 'Missing host parameter');
        return;
    }

    if (!ALLOWED_HOSTS.includes(host.toLowerCase())) {
        console.error(`Connection rejected: Host '${host}' is not in the allowlist.`);
        ws.close(1008, 'Host not allowed');
        return;
    }

    console.log(`[PROXY] Connecting to ${host}:${port}...`);
    
    const tcpSocket = new Socket();

    tcpSocket.on('connect', () => {
        console.log(`[PROXY] Connected to ${host}:${port}`);
        // Optionally notify the client that connection is established if needed,
        // but raw telnet just sends data.
    });

    tcpSocket.on('data', (data) => {
        if (ws.readyState === ws.OPEN) {
            ws.send(data);
        }
    });

    tcpSocket.on('close', () => {
        console.log(`[PROXY] Disconnected from ${host}:${port}`);
        if (ws.readyState === ws.OPEN) {
            ws.close();
        }
    });

    tcpSocket.on('error', (err) => {
        console.error(`[PROXY] TCP Error (${host}:${port}):`, err.message);
        if (ws.readyState === ws.OPEN) {
            ws.close(1011, 'TCP Connection Error');
        }
    });

    ws.on('message', (message) => {
        if (!tcpSocket.destroyed) {
            tcpSocket.write(message);
        }
    });

    ws.on('close', () => {
        console.log(`[PROXY] WebSocket closed by client`);
        if (!tcpSocket.destroyed) {
            tcpSocket.destroy();
        }
    });

    ws.on('error', (err) => {
        console.error(`[PROXY] WebSocket Error:`, err.message);
        if (!tcpSocket.destroyed) {
            tcpSocket.destroy();
        }
    });

    // Initiate TCP connection
    tcpSocket.connect(port, host);
});

server.listen(PORT, () => {
    console.log(`[PROXY] Listening on port ${PORT}...`);
    console.log(`[PROXY] Point KBBS Web Client to ws://localhost:${PORT}/?host=<target>&port=<port>`);
});
