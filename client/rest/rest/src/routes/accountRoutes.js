import catapult from 'catapult-sdk';
import routeResultTypes from './routeResultTypes';
import routeUtils from './routeUtils';

const address = catapult.model.address;
const convert = catapult.utils.convert;

export default {
	register: (server, db) => {
		function sendAccountOrNotFound(id, res, next) {
			return routeUtils.sendEntityOrNotFound(id, routeResultTypes.account, res, next);
		}

		function sendTransactions(id, res, next) {
			return routeUtils.sendEntities(id, routeResultTypes.transfer, res, next);
		}

		server.get('/account/address/:address', (req, res, next) => {
			const encodedAddress = routeUtils.parseArgument(req.params, 'address', address.stringToAddress);
			return db.accountGet(encodedAddress)
				.then(sendAccountOrNotFound(req.params.address, res, next));
		});

		server.get('/account/key/:publicKey', (req, res, next) => {
			const publicKey = routeUtils.parseArgument(req.params, 'publicKey', convert.hexToUint8);
			return db.accountGetFromPublicKey(publicKey)
				.then(sendAccountOrNotFound(req.params.publicKey, res, next));
		});

		const Account_Transactions_Route_To_Db_Map = [
			{ path: '', handler: 'accountTransactionsAll' },
			{ path: '/incoming', handler: 'accountTransactionsIncoming' },
			{ path: '/outgoing', handler: 'accountTransactionsOutgoing' },
			{ path: '/unconfirmed', handler: 'accountTransactionsUnconfirmed' }
		];

		for (const elem of Account_Transactions_Route_To_Db_Map) {
			server.get(`/account/key/:publicKey/transactions${elem.path}`, (req, res, next) => {
				const publicKey = routeUtils.parseArgument(req.params, 'publicKey', convert.hexToUint8);
				const pagingOptions = routeUtils.parsePagingArguments(req.params);
				return db[elem.handler](publicKey, pagingOptions.id, pagingOptions.pageSize)
					.then(sendTransactions('publicKey', res, next));
			});
		}
	}
};
