/** @module plugins/transfer */
import EntityType from '../model/EntityType';
import ModelType from '../model/ModelType';
import sizes from '../modelBinary/sizes';

const constants = { sizes };

/**
 * Creates a transfer plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport('transfer', {
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
					transaction.message.payload = parser.buffer(messageSize - 1);
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

				if (0 < payloadSize) {
					serializer.writeUint8(transaction.message.type);
					serializer.writeBuffer(transaction.message.payload);
				}

				if (0 < numMosaics) {
					for (const mosaic of transaction.mosaics) {
						serializer.writeUint64(mosaic.id);
						serializer.writeUint64(mosaic.amount);
					}
				}
			}
		});
	}
};
