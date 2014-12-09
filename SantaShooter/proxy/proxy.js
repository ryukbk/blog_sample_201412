#!/usr/bin/env node

var WebSocketServer = require('ws').Server
var wss = new WebSocketServer({port: 31337});

wss.on('connection', function(ws) {
	console.log(wss.clients.length.toString(), 'clients are connected');

	if (wss.clients.length == 1) {
		console.log("Server connected");
		ws.name = "Server";
		ws.send(JSON.stringify({m: 0, role: 0}));
	} else if (wss.clients.length == 2) {
		console.log("Player 1 connected");
		ws.name = "Player 1";
		ws.send(JSON.stringify({m: 0, role: 1}));
	} else if (wss.clients.length == 3) {
		console.log("Player 2 connected");
		ws.name = "Player 2";
		ws.send(JSON.stringify({m: 0, role: 2}));
	}

	ws.on('message', function(message) {
		if (ws === wss.clients[0]) {
			var json = JSON.parse(message);
			if (json.d == 0) {
				wss.clients[1].send(message);
			} else if (json.d == 1) {
				wss.clients[2].send(message);
			}
		} else { // client to server
			wss.clients[0].send(message);
		}
	});

	ws.on('close', function close() {
		console.log(ws.name, 'disconnected');
	});
});

