const catapult = require('catapult-sdk');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');

const { convert } = catapult.utils;

module.exports = {
	register: (server, db, services) => {
		const transactionSender = routeUtils.createSender(routeResultTypes.transfer);

		routeUtils.addGetPostDocumentRoutes(
			server,
			routeUtils.createSender(routeResultTypes.account),
			{ base: '/account', singular: 'accountId', plural: 'accountIds' },
			params => db.accountsByIds(params.map(tuple => ({ [tuple[0]]: tuple[1] }))),
			'accountId'
		);

		const transactionStates = [
			{ dbPostfix: 'All', routePostfix: '' },
			{ dbPostfix: 'Incoming', routePostfix: '/incoming' },
			{ dbPostfix: 'Outgoing', routePostfix: '/outgoing' },
			{ dbPostfix: 'Unconfirmed', routePostfix: '/unconfirmed' }
		];

		transactionStates.concat(services.config.transactionStates).forEach(state => {
			server.get(`/account/:publicKey/transactions${state.routePostfix}`, (req, res, next) => {
				const publicKey = routeUtils.parseArgument(req.params, 'publicKey', convert.hexToUint8);
				const pagingOptions = routeUtils.parsePagingArguments(req.params);
				return db[`accountTransactions${state.dbPostfix}`](publicKey, pagingOptions.id, pagingOptions.pageSize)
					.then(transactionSender.sendArray('publicKey', res, next));
			});
		});
	}
};
