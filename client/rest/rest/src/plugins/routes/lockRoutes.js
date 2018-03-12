const routeUtils = require('../../routes/routeUtils');

module.exports = {
	register: (server, db) => {
		server.get('/account/:accountId/lock/hash', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');

			return db.hashLocksByAccounts(type, [accountId])
				.then(routeUtils.createSender('hashLockInfo').sendOne(req.params.accountId, res, next));
		});

		server.get('/account/:accountId/lock/secret', (req, res, next) => {
			const [type, accountId] = routeUtils.parseArgument(req.params, 'accountId', 'accountId');

			return db.secretLocksByAccounts(type, [accountId])
				.then(routeUtils.createSender('secretLockInfo').sendOne(req.params.accountId, res, next));
		});
	}
};
