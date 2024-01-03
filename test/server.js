const http = require('http');
const DEFAULT_PORT = 4895;
const servers = [];
const ansi_console_green = '\x1b[32m';
const ansi_console_reset = '\x1b[0m';

function createServer(port) {
    const parsedPort = parseInt(port, 10);
    if (isNaN(parsedPort) || parsedPort <= 0 || parsedPort > 65535)
        throw new Error(`Invalid port number: ${port}`);
    const server = http.createServer((req, res) => {
        const serverAddr = req.socket.localAddress;
        const clientAddr = req.socket.remoteAddress;
        const clientPort = req.socket.remotePort;
        console.log(
            `Request from ${clientAddr}:${ansi_console_green}${clientPort}${ansi_console_reset} to` +
            `${serverAddr}:${ansi_console_green}${parsedPort}${ansi_console_reset}\n` +
            `Method: ${req.method}\n` +
            `URL: ${req.url}\n` +
            `Headers: ${JSON.stringify(req.headers)}`
        );
        res.writeHead(200, {'Content-Type': 'text/plain'});
        res.end(`This is server from ${parsedPort} port!\n`);
    });
    server.listen(parsedPort, () => {
        console.log(`Server running at http://localhost:${parsedPort}/`);
    });
    servers.push(server);
}

function closeServer(index) {
    if (index >= 0 && index < servers.length) {
        servers[index].close();
        console.log(`Server at index ${index} closed.`);
    } else {
        console.error(`Invalid server index: ${index}`);
    }
}

const ports = process.argv.slice(2);
if (ports.length === 0)
    createServer(DEFAULT_PORT);
else
    ports.forEach(port => createServer(port));