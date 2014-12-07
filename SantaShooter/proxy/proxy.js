#!/usr/bin/env node

var WebSocketServer = require('ws').Server

var wss = new WebSocketServer({port: 31337});

wss.on('connection', function(ws) {
	console.log(wss.clients.length.toString(), 'clients are connected');

	ws.on('message', function(message) {
console.log(message);

		/*if (wss.clients.length >= 2) {
			for (var i in wss.clients) {
				if (ws !== wss.clients[i]) {
					wss.clients[i].send(message);
				}
			}
		}*/
	});
});

