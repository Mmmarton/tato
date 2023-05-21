const net = require('net');
const { Observable, Subject } = require('rxjs');

/**
 *
 * @param {string} host
 * @param {string} port
 * @returns {Observable<{stream:Observable<string>, write:(string)=>void, close:()=>void}>}
 */
function startTCPServer(host, port) {
  const server = net.createServer();

  server.listen(port, host, () => {
    console.log(`TCP server running on: ${host}:${port}`);
  });

  return new Observable((observer) => {
    server.addListener('connection', (socket) => {
      const subject = createSocketSubject(socket);
      observer.next({
        stream: subject.asObservable(),
        write: (data) => {
          if (data) {
            socket.write(data);
          }
        },
        close: () => {
          subject.complete();
          socket.resetAndDestroy();
        },
      });
    });
  });
}

function splitData(data) {
  if (!data) {
    return [];
  }

  let dataArray = [];
  let lastArray = [];

  let text = `${data}`;
  for (let i = 0; i < text.length; i++) {
    if (text[i] === '\n') {
      dataArray.push(lastArray);
      lastArray = [];
    } else {
      lastArray.push(text[i]);
    }
  }

  if (lastArray.length) {
    dataArray.push(lastArray);
  }

  return dataArray;
}

function createSocketSubject(socket) {
  const subject = new Subject();

  socket
    .on('data', (data) => {
      let dataArrays = splitData(data);
      dataArrays.forEach((array) => subject.next(array.join('')));
    })
    .on('end', () => {
      subject.next([-1]);
      subject.complete();
    })
    .on('error', (error) => {
      console.error(`TCP error: ${error}`);
      subject.next([-1]);
      subject.complete();
    });
  socket.setKeepAlive(true, 1000);

  return subject;
}

module.exports = { startTCPServer };
