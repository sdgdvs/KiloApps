const { WebSocketServer } = require('ws');
const { Socket } = require('net');
const http = require('http');

const PORT = process.env.PORT || 8080;

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

const ALLOWED_PORTS = [23, 1337, 2323, 2525, 8023, 9023, 44510];

const server = http.createServer((req, res) => {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('KBBS WebSocket-to-Telnet Proxy is running.\n');
});

const wss = new WebSocketServer({ server });

wss.on('connection', (ws, req) => {
    const url = new URL(req.url, `http://${req.headers.host}`);
    let host = url.searchParams.get('host');
    let port = parseInt(url.searchParams.get('port') || '23', 10);

    const origin = req.headers.origin || '';
    const isLocalhost = origin.startsWith('http://localhost:') || origin.startsWith('http://127.0.0.1:') || origin.startsWith('https://localhost:') || origin.startsWith('https://127.0.0.1:');
    const isKiloApps = origin === 'https://kiloapps.web.app' || origin === 'https://kiloapps.firebaseapp.com' || (origin.startsWith('https://kiloapps--') && (origin.endsWith('.web.app') || origin.endsWith('.firebaseapp.com')));
    const isAllowedOrigin = isLocalhost || isKiloApps || !origin || origin === 'null';

    if (!isAllowedOrigin) {
        console.error(`Connection rejected: Invalid Origin '${origin}'`);
        ws.close(1008, 'Origin not allowed');
        return;
    }

    if (!host) {
        console.error('Connection rejected: Missing host parameter');
        ws.close(1008, 'Missing host parameter');
        return;
    }

    if (host.includes(':')) {
        const parts = host.split(':');
        host = parts[0];
        port = parseInt(parts[1], 10) || port;
    }

    if (!ALLOWED_HOSTS.includes(host.toLowerCase())) {
        console.error(`[PROXY] Connection rejected: Host '${host}' is not in the allowlist.`);
        ws.close(1008, 'Host not allowed');
        return;
    }

    if (!Number.isInteger(port) || !ALLOWED_PORTS.includes(port)) {
        console.error(`[PROXY] Connection rejected: Port '${port}' is not in the allowlist.`);
        ws.close(1008, 'Port not allowed');
        return;
    }

    console.log(`[PROXY] Connect request from ${origin} to ${host}:${port}...`);

    console.log(`[PROXY] Connecting to ${host}:${port}...`);
    
    const tcpSocket = new Socket();

    tcpSocket.on('connect', () => {
        console.log(`[PROXY] Connected to ${host}:${port}`);
        // Optionally notify the client that connection is established if needed,
        // but raw telnet just sends data.
    });

    let idleTimer;
    const IDLE_TIMEOUT_MS = 5 * 60 * 1000; // 5 minutes

    function resetIdleTimer() {
        if (idleTimer) clearTimeout(idleTimer);
        idleTimer = setTimeout(() => {
            console.log(`[PROXY] Idle timeout reached for ${host}:${port}, disconnecting...`);
            if (ws.readyState === ws.OPEN) {
                ws.close(1000, 'Idle timeout');
            }
            if (!tcpSocket.destroyed) {
                tcpSocket.destroy();
            }
        }, IDLE_TIMEOUT_MS);
    }

    // Start the timer immediately upon connection
    resetIdleTimer();

    tcpSocket.on('data', (data) => {
        resetIdleTimer();
        if (ws.readyState === ws.OPEN) {
            ws.send(data);
        }
    });

    tcpSocket.on('close', () => {
        if (idleTimer) clearTimeout(idleTimer);
        console.log(`[PROXY] Disconnected from ${host}:${port}`);
        if (ws.readyState === ws.OPEN) {
            ws.close();
        }
    });

    tcpSocket.on('error', (err) => {
        if (idleTimer) clearTimeout(idleTimer);
        console.error(`[PROXY] TCP Error (${host}:${port}):`, err.message);
        if (ws.readyState === ws.OPEN) {
            ws.close(1011, 'TCP Connection Error');
        }
    });

    ws.on('message', (message) => {
        resetIdleTimer();
        if (!tcpSocket.destroyed) {
            tcpSocket.write(message);
        }
    });

    ws.on('close', () => {
        if (idleTimer) clearTimeout(idleTimer);
        console.log(`[PROXY] WebSocket closed by client`);
        if (!tcpSocket.destroyed) {
            tcpSocket.destroy();
        }
    });

    ws.on('error', (err) => {
        if (idleTimer) clearTimeout(idleTimer);
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
