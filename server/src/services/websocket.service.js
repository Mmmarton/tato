const WebSocket = require('ws');
const { Observable, Subject } = require('rxjs');

/**
 *
 * @param {string} host
 * @param {string} port
 * -@returns {Observable<{stream:Observable<string>, write:(string)=>void}>}
 */
function startWebSocketService(port) {
  const websocketServer = new WebSocket.Server({
    port: port,
  });

  websocketServer.on('open', () => {
    console.info(`Websocket server running on port: ${port}`);
  });

  return new Observable((observer) => {
    websocketServer.on('connection', (client) => {
      observer.next({
        stream: createWebSocketStream(client),
        write: (data) => client.send(data),
      });
    });
  });
}

function createWebSocketStream(client) {
  const subject = new Subject();

  client.on('error', (error) => {
    console.error(`Websocket error: ${error}`);
    LogService.logger({
      level: 'error',
      message: 'Create WebSocketStream failed!',
    });
  });

  client.on('message', (data) => {
    subject.next(data.toString());
  });

  client.on('close', () => {
    subject.complete();
  });

  client.on('error', (error) => {
    console.error(`Websocket error: ${error}`);
    LogService.logger({
      level: 'error',
      message: 'Create WebSocketStream failed!',
    });
    subject.complete();
  });

  return subject.asObservable();
}

module.exports = { startWebSocketService };
