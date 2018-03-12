/** @module plugins/aggregate */
const catapult = require('catapult-sdk');
const aggregateRoutes = require('./routes/aggregateRoutes');

const { BinaryParser } = catapult.parser;

/**
 * Creates an aggregate plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
module.exports = {
	createDb: () => {},

	registerTransactionStates: states => {
		states.push({ friendlyName: 'partial', dbPostfix: 'Partial', routePostfix: '/partial' });
	},

	registerMessageChannels: builder => {
		builder.add('partialAdded', 'p', 'transaction');
		builder.add('partialRemoved', 'q', 'transactionHash');
		builder.add('cosignature', 'c', (codec, emit) => (topic, buffer) => {
			const parser = new BinaryParser();
			parser.push(buffer);

			const signer = parser.buffer(catapult.constants.sizes.signer);
			const signature = parser.buffer(catapult.constants.sizes.signature);
			const parentHash = parser.buffer(catapult.constants.sizes.hash);
			emit({ type: 'aggregate.cosignature', payload: { signer, signature, parentHash } });
		});
	},

	registerRoutes: (...args) => {
		aggregateRoutes.register(...args);
	}
};
