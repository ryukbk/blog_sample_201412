#!/usr/bin/env node

var WebSocketServer = require('ws').Server
var wss = new WebSocketServer({port: 31337});

wss.on('connection', function(ws) {
	console.log(wss.clients.length.toString(), 'clients are connected');

	if (wss.clients.length == 1) {
		console.log("Server connected");
		ws.name = "Server";
		ws.send(JSON.stringify({o: 0, d: 1, t: 0, a: 0}));
	} else if (wss.clients.length == 2) {
		console.log("Player 1 connected");
		ws.name = "Player 1";
		ws.send(JSON.stringify({o: 0, d: 2, t: 0, a: 0}));
	} else if (wss.clients.length == 3) {
		console.log("Player 2 connected");
		ws.name = "Player 2";
		ws.send(JSON.stringify({o: 0, d: 3, t: 0, a: 0}));
	}

	ws.on('message', function(message) {
		if (ws === wss.clients[0]) {
			var json = JSON.parse(message);
			if (json.o != 4) {
				console.log(ws.name + ": " + message);
			}

			if (json.d == 2) {
				if (wss.clients[1] != null) {
					wss.clients[1].send(message);
				}
			} else if (json.d == 3) {
				if (wss.clients[2] != null) {
					wss.clients[2].send(message);
				}
			} else if (json.d == 4) {
				if (wss.clients[1] != null) {
					wss.clients[1].send(message);
				}
				if (wss.clients[2] != null) {
					wss.clients[2].send(message);
				}
			}
		} else { // client to server
			console.log(ws.name + ": " + message);
			wss.clients[0].send(message);
		}
	});

	ws.on('close', function close() {
		console.log(ws.name, 'disconnected');
	});
});

