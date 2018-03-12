const catapult = require('catapult-sdk');
const routeResultTypes = require('./routeResultTypes');
const routeUtils = require('./routeUtils');

const { convert } = catapult.utils;
const { PacketType } = catapult.packet;

const constants = {
	sizes: {
		hash: 64,
		objectId: 24
	}
};

const parseObjectId = str => {
	if (!convert.isHexString(str))
		throw Error('must be 12-byte hex string');

	return str;
};

module.exports = {
	register: (server, db, services) => {
		const sender = routeUtils.createSender(routeResultTypes.transfer);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transaction', packetType: PacketType.pushTransactions },
			params => routeUtils.parseArgument(params, 'payload', convert.hexToUint8)
		);

		routeUtils.addGetPostDocumentRoutes(
			server,
			sender,
			{ base: '/transaction', singular: 'transactionId', plural: 'transactionIds' },
			// params has already been converted by a parser below, so it is: string - in case of objectId, Uint8Array - in case of hash
			params => (('string' === typeof params[0]) ? db.transactionsByIds(params) : db.transactionsByHashes(params)),
			(transactionId, index, array) => {
				if (0 < index && array[0].length !== transactionId.length)
					throw Error(`all ids must be homogeneous, element ${index}`);

				if (constants.sizes.objectId === transactionId.length)
					return parseObjectId(transactionId);
				else if (constants.sizes.hash === transactionId.length)
					return convert.hexToUint8(transactionId);

				throw Error(`invalid length of transaction id '${transactionId}'`);
			}
		);
	}
};
