const catapult = require('catapult-sdk');
const dbFacade = require('./dbFacade');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');

const { convert } = catapult.utils;
const { constants } = catapult;

module.exports = {
	register: (server, db, services) => {
		routeUtils.addGetPostDocumentRoutes(
			server,
			routeUtils.createSender(routeResultTypes.transactionStatus),
			{
				base: '/transaction', singular: 'hash', plural: 'hashes', postfixes: { singular: 'status', plural: 'statuses' }
			},
			params => dbFacade.transactionStatusesByHashes(db, params, services.config.transactionStates),
			hash => {
				if (2 * constants.sizes.hash === hash.length)
					return convert.hexToUint8(hash);

				throw Error(`invalid length of hash '${hash.length}'`);
			}
		);
	}
};
