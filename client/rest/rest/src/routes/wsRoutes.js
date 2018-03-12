module.exports = {
	register: (server, db, services) => {
		server.ws('/ws', {
			newChannel: (channel, sender) => {
				services.zmqService.on(channel, message => sender.send(message));
				services.zmqService.on(`${channel}.close`, () => sender.close());
			},
			removeChannel: channel => services.zmqService.removeAllListeners(channel)
		});
	}
};
