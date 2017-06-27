/** @module plugins/aggregate */
import EntityType from '../model/EntityType';
import ModelType from '../model/ModelType';
import embeddedEntityCodec from '../modelBinary/embeddedEntityCodec';
import sizes from '../modelBinary/sizes';
import SerializedSizeCalculator from '../serializer/SerializedSizeCalculator';

const constants = { sizes: {} };
Object.assign(constants.sizes, sizes, {
	aggregate: 4,
	embedded: 40,
	cosignature: sizes.signer + sizes.signature
});

function createSubTransactionCodec(txCodecs) {
	function getTxCodec(type) {
		// unlike in block case (handled by ModelCodecBuilder), don't fallback to unknown transaction type
		const txCodec = txCodecs[type];
		if (!txCodec)
			throw Error(`error unsupported transaction type (${type}) in aggregate`);

		return txCodec;
	}

	function serializeAll(transaction, serializer) {
		const codecs = [embeddedEntityCodec, getTxCodec(transaction.type)];
		for (const codec of codecs)
			codec.serialize(transaction, serializer);
	}

	// notice that the subTxCodec is not conformant and is slightly different from other codecs
	const subTxCodec = {
		size: transaction => {
			const sizeCalculator = new SerializedSizeCalculator();
			serializeAll(transaction, sizeCalculator);
			return sizeCalculator.size() + 4; // include size of size field itself
		},

		deserialize: parser => {
			const size = parser.uint32();
			const entity = embeddedEntityCodec.deserialize(parser);

			const txCodec = getTxCodec(entity.type);
			Object.assign(entity, txCodec.deserialize(parser));
			return { size, entity };
		},

		serialize: (transaction, serializer, size) => {
			serializer.writeUint32(size);
			serializeAll(transaction, serializer);
		}
	};

	return subTxCodec;
}

function requireCodecs(txCodecs) {
	// this check causes rejection of embedded aggregates because aggregate codec intentionally does not forward tx codecs to
	// sub transaction codecs
	if (undefined === txCodecs)
		throw Error('aggregate transaction is not embeddable');
}

/**
 * Creates an aggregate plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport('aggregate', {
			transactions: { type: ModelType.array, schemaName: 'transactionWithMetadata' },
			cosignatures: { type: ModelType.array, schemaName: 'aggregate.cosignature' }
		});

		builder.addSchema('aggregate.cosignature', {
			signer: ModelType.binary,
			signature: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.aggregate, {
			deserialize: (parser, size, txCodecs) => {
				requireCodecs(txCodecs);

				if (size < constants.sizes.aggregate)
					throw Error('aggregate must contain complete aggregate header');

				const payloadSize = parser.uint32();
				if (size < payloadSize + constants.sizes.aggregate)
					throw Error('aggregate must contain complete payload');

				// 1. deserialize transactions
				const transaction = {};
				if (0 < payloadSize) {
					transaction.transactions = [];

					const txCodec = createSubTransactionCodec(txCodecs);
					let processedSize = 0;
					while (processedSize < payloadSize) {
						const subTransaction = txCodec.deserialize(parser);
						transaction.transactions.push(subTransaction.entity);
						processedSize += subTransaction.size;

						if (subTransaction.size < constants.sizes.embedded)
							throw Error('sub transaction must contain complete transaction header');
					}
				}

				// 2. deserialize cosignatures
				const numCosignatures = (size - payloadSize - constants.sizes.aggregate) / constants.sizes.cosignature;
				if (numCosignatures !== (numCosignatures | 0))
					throw Error('aggregate cannot have partial cosignatures');

				if (0 < numCosignatures) {
					transaction.cosignatures = [];
					for (let i = 0; i < numCosignatures; ++i) {
						const cosignature = {};
						cosignature.signer = parser.buffer(constants.sizes.signer);
						cosignature.signature = parser.buffer(constants.sizes.signature);
						transaction.cosignatures.push(cosignature);
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer, txCodecs) => {
				requireCodecs(txCodecs);

				// 1. calculate payload size
				const txCodec = createSubTransactionCodec(txCodecs);
				const transactions = transaction.transactions || [];
				const subTransactionSizes = [];

				let payloadSize = 0;
				for (const subTransaction of transactions) {
					const subTransactionSize = txCodec.size(subTransaction);
					subTransactionSizes.push(subTransactionSize);
					payloadSize += subTransactionSize;
				}

				serializer.writeUint32(payloadSize);

				// 2. serialize transactions
				let i = 0;
				for (const subTransaction of transactions)
					txCodec.serialize(subTransaction, serializer, subTransactionSizes[i++]);

				// 3. serialize cosignatures
				if (transaction.cosignatures) {
					for (const cosignature of transaction.cosignatures) {
						serializer.writeBuffer(cosignature.signer);
						serializer.writeBuffer(cosignature.signature);
					}
				}
			}
		});
	}
};
