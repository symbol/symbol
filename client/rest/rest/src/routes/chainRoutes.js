import routeResultTypes from './routeResultTypes';

function curryStripProperties(properties, next) {
	return chainInfo => {
		for (const property of properties)
			delete chainInfo[property];

		next(chainInfo);
	};
}

function currySend(db, res, next) {
	return chainInfo => {
		res.send({ payload: chainInfo, type: routeResultTypes.chainInfo });
		next();
	};
}

export default {
	register: (server, db) => {
		server.get('/chain/height', (req, res, next) =>
			db.chainInfo()
				.then(curryStripProperties(['scoreLow', 'scoreHigh'], currySend(db, res, next))));

		server.get('/chain/score', (req, res, next) =>
			db.chainInfo()
				.then(curryStripProperties(['height'], currySend(db, res, next))));
	}
};
