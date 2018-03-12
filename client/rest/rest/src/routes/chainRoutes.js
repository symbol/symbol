const routeResultTypes = require('./routeResultTypes');

const curryStripProperties = (properties, next) => chainInfo => {
	properties.forEach(property => {
		delete chainInfo[property];
	});

	next(chainInfo);
};

const currySend = (db, res, next) => chainInfo => {
	res.send({ payload: chainInfo, type: routeResultTypes.chainInfo });
	next();
};

module.exports = {
	register: (server, db) => {
		server.get('/chain/height', (req, res, next) =>
			db.chainInfo().then(curryStripProperties(['scoreLow', 'scoreHigh'], currySend(db, res, next))));

		server.get('/chain/score', (req, res, next) =>
			db.chainInfo().then(curryStripProperties(['height'], currySend(db, res, next))));
	}
};
