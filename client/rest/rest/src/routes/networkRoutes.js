module.exports = {
	register: (server, db, services) => {
		server.get('/network', (req, res, next) => {
			// forward entire config network section without formatting
			res.send(services.config.network);
			next();
		});
	}
};
