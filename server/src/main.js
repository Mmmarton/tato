const path = require('path');
const { startTCPServer } = require('./services/tcp.service');
const { startWebSocketService } = require('./services/websocket.service');
const dotenv = require('dotenv');
const cors = require('cors');
const { finalize } = require('rxjs');
const express = require('express');
const { logIn, isLoggedIn } = require('./services/auth.service');

let wsClient;
let tcpConnection;

dotenv.config();
startWebServers();
setupRoutes();

function setupRoutes() {
  const app = express();
  app.use(cors());
  app.use(express.json());
  app.use(express.static(path.join(__dirname, '../../interface/build')));

  app.post('/login', (req, resp) => {
    if (!req.body) {
      resp.status(400).json({ error: 'Missing request body.' });
    }

    const { username, password } = req.body;
    const token = logIn(username, password);

    resp.json({ token });
  });

  app.get('/healthcheck', (req, resp) => {
    resp.json({ isLoggedIn: isLoggedIn(req.header('Authorization')) });
  });

  app.listen(process.env.HTTP_PORT, () =>
    console.info(
      `HTTP server listening on: ${process.env.HTTP_HOST}:${process.env.HTTP_PORT}`
    )
  );
}

function startWebServers() {
  startTCPServer(process.env.TCP_HOST, process.env.TCP_PORT).subscribe(
    (connection) => {
      console.info(`TCP new connection.`);
      tcpConnection = connection;
      wsClient?.write(`hello`);

      connection.stream
        .pipe(
          finalize(() => {
            if (connection === tcpConnection) {
              tcpConnection = null;
              wsClient?.write(`bye`);
            }
          })
        )
        .subscribe((message) => {
          console.log(message);
          if (wsClient) {
            wsClient?.write(`beep`);
          }
        });
    }
  );

  startWebSocketService(process.env.WEBSOCKET_PORT).subscribe((connection) => {
    console.info(`WS new connection.`);
    wsClient = connection;

    wsClient.stream.subscribe(console.log);

    if (tcpConnection) {
      wsClient.write(`hello`);
    }
  });
}
