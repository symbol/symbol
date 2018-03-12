const catapult = require('catapult-sdk');
const networkTime = require('../utils/networkTime');
const random = require('../utils/random');

const { uint64 } = catapult.utils;

const createTransaction = (options, type) => ({
	signature: new Uint8Array(catapult.constants.sizes.signature),
	signer: options.signerPublicKey,
	version: (options.networkId << 8) + 3,
	type,
	fee: uint64.fromUint(0),
	deadline: uint64.fromUint(networkTime.getNetworkTime() + (60 * 60 * 1000))
});

module.exports = {
	createRandomTransfer: (options, recipientSelector) => Object.assign(createTransaction(options, catapult.model.EntityType.transfer), {
		recipient: recipientSelector(),
		message: {
			type: 0,
			payload: Buffer.from(uint64.toHex(uint64.fromUint(options.transferId)), 'hex')
		},
		mosaics: [
			{ id: [0xD95FCF29, 0xD525AD41],	amount: uint64.fromUint(random.uint32(1000000)) }
		]
	}),

	createAggregateTransaction: (options, transactions) => Object.assign(
		createTransaction(options, catapult.model.EntityType.aggregateComplete),
		{ transactions }
	)
};
