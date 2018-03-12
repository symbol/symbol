/** @module plugins/transfer */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

/**
 * Creates a transfer plugin.
 * @type {module:plugins/CatapultPlugin}
 */
module.exports = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.transfer, {
			recipient: ModelType.binary,
			message: { type: ModelType.object, schemaName: 'transfer.message' },
			mosaics: { type: ModelType.array, schemaName: 'mosaic' }
		});
		builder.addSchema('transfer.message', {
			payload: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.transfer, {
			deserialize: parser => {
				const transaction = {};
				transaction.recipient = parser.buffer(constants.sizes.addressDecoded);

				const messageSize = parser.uint16();
				const numMosaics = parser.uint8();

				if (0 < messageSize) {
					transaction.message = {};
					transaction.message.type = parser.uint8();
					transaction.message.payload = 1 < messageSize ? parser.buffer(messageSize - 1) : [];
				}

				if (0 < numMosaics) {
					transaction.mosaics = [];
					while (transaction.mosaics.length < numMosaics) {
						const id = parser.uint64();
						const amount = parser.uint64();
						transaction.mosaics.push({ id, amount });
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeBuffer(transaction.recipient);

				let payloadSize = 0;
				if (transaction.message) {
					payloadSize = transaction.message.payload.length;
					serializer.writeUint16(payloadSize + 1);
				} else {
					serializer.writeUint16(0);
				}

				const numMosaics = transaction.mosaics ? transaction.mosaics.length : 0;
				serializer.writeUint8(numMosaics);

				if (transaction.message) {
					serializer.writeUint8(transaction.message.type);

					if (0 < payloadSize)
						serializer.writeBuffer(transaction.message.payload);
				}

				if (0 < numMosaics) {
					transaction.mosaics.forEach(mosaic => {
						serializer.writeUint64(mosaic.id);
						serializer.writeUint64(mosaic.amount);
					});
				}
			}
		});
	}
};
