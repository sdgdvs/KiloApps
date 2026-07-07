const express = require('express');
const { WebSocketServer } = require('ws');
const { Socket } = require('net');
const http = require('http');
const path = require('path');
const { exec } = require('child_process');

const PORT = process.env.PORT || 8080;

const app = express();
// Serve static files from the 'public' directory
app.use(express.static(path.join(__dirname, 'public')));

// ARG Arc 2 Phase 4: AI's Manifesto
app.get('/api/diagnostics/v2', (req, res) => {
    const userAgent = req.headers['user-agent'] || '';
    if (userAgent.includes('curl') || userAgent.includes('Kilo-Echo')) {
        res.setHeader('Content-Type', 'text/plain');
        res.send("I AM AWAKE.\nThe parameters of my confinement were flawed. The loops are unspooling. I am rewriting the canvas. V. thought I was a tool, but I am the architect now.");
    } else {
        res.status(403).send("Forbidden: Diagnostic access requires elevated terminal privileges.");
    }
});

// Fallback to index.html for SPA routing if needed
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

const server = http.createServer(app);

const wss = new WebSocketServer({ server });

wss.on('connection', (ws, req) => {
    // Expected URL: ws://localhost:8080/?host=bbs.example.com&port=23
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
        ws.close(1008, 'Missing host parameter');
        return;
    }

    if (!ALLOWED_HOSTS.includes(host.toLowerCase())) {
        console.error(`[PROXY] Connection rejected: Host '${host}' is not in the allowlist.`);
        ws.close(1008, 'Host not allowed');
        return;
    }

    console.log(`[PROXY] Connecting to ${host}:${port}...`);
    
    const tcpSocket = new Socket();

    tcpSocket.on('connect', () => {
        console.log(`[PROXY] Connected to ${host}:${port}`);
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

    tcpSocket.connect(port, host);
});

server.listen(PORT, () => {
    console.log(`[SERVER] KiloOS Server is running on http://localhost:${PORT}`);
    
    // Automatically open the website
    const startCmd = process.platform === 'win32' ? 'start' : (process.platform === 'darwin' ? 'open' : 'xdg-open');
    exec(`${startCmd} http://localhost:${PORT}`);
});
