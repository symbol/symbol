const catapult = require('catapult-sdk');
const routeUtils = require('../../routes/routeUtils');

const { convert } = catapult.utils;
const { PacketType } = catapult.packet;

module.exports = {
	register: (server, db, services) => {
		const parseHexParam = (params, key) => routeUtils.parseArgument(params, key, convert.hexToUint8);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transaction/partial', packetType: PacketType.pushPartialTransactions },
			params => parseHexParam(params, 'payload')
		);

		routeUtils.addPutPacketRoute(
			server,
			services.connections,
			{ routeName: '/transaction/cosignature', packetType: PacketType.pushDetachedCosignatures },
			params => Buffer.concat(['signer', 'signature', 'parentHash'].map(key => parseHexParam(params, key)))
		);
	}
};
