/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @module plugins/aggregate */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const embeddedEntityCodec = require('../modelBinary/embeddedEntityCodec');
const sizes = require('../modelBinary/sizes');
const SerializedSizeCalculator = require('../serializer/SerializedSizeCalculator');

const constants = { sizes: {} };
Object.assign(constants.sizes, sizes, {
	aggregate: 128 + 32 + 4 + 4, // size passed into deserialize includes full transaction size (even previously processed parts)
	embedded: 48,
	cosignature: 8 + sizes.signerPublicKey + sizes.signature
});

const createSubTransactionCodec = txCodecs => {
	const getTxCodec = type => {
		// unlike in block case (handled by ModelCodecBuilder), don't fallback to unknown transaction type
		const txCodec = txCodecs[type];
		if (!txCodec)
			throw Error(`error unsupported transaction type (${type}) in aggregate`);

		return txCodec;
	};

	const serializeAll = (transaction, serializer) => {
		const codecs = [embeddedEntityCodec, getTxCodec(transaction.type)];
		codecs.forEach(codec => {
			codec.serialize(transaction, serializer);
		});
	};

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
};

const requireCodecs = txCodecs => {
	// this check causes rejection of embedded aggregates because aggregate codec intentionally does not forward tx codecs to
	// sub transaction codecs
	if (undefined === txCodecs)
		throw Error('aggregate transaction is not embeddable');
};

// after every inner transaction in an aggregate there's padding up to 8 bytes
const innerAggregateTxPaddingSize = innerTransactionSize => {
	const alignment = 8;
	return 0 === innerTransactionSize % alignment ? 0 : alignment - (innerTransactionSize % alignment);
};

/**
 * Creates an aggregate plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const aggregatePlugin = {
	registerSchema: builder => {
		const aggregateSchema = {
			transactionsHash: ModelType.binary,
			transactions: { type: ModelType.array, schemaName: 'transactionWithMetadata' },
			cosignatures: { type: ModelType.array, schemaName: 'aggregate.cosignature' }
		};

		builder.addTransactionSupport(EntityType.aggregateComplete, aggregateSchema);
		builder.addTransactionSupport(EntityType.aggregateBonded, aggregateSchema);

		builder.addSchema('aggregate.cosignature', {
			version: ModelType.uint64,
			signerPublicKey: ModelType.binary,
			signature: ModelType.binary,
			parentHash: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		const aggregateBuilder = {
			deserialize: (parser, size, txCodecs) => {
				requireCodecs(txCodecs);

				const transaction = {};
				transaction.transactionsHash = parser.buffer(constants.sizes.hash256);

				if (size < constants.sizes.aggregate)
					throw Error('aggregate must contain complete aggregate header');

				const payloadSize = parser.uint32();
				if (size < payloadSize + constants.sizes.aggregate)
					throw Error('aggregate must contain complete payload');

				transaction.aggregateTransactionHeader_Reserved1 = parser.uint32();

				// 1. deserialize transactions
				if (0 < payloadSize) {
					transaction.transactions = [];

					const txCodec = createSubTransactionCodec(txCodecs);
					let processedSize = 0;
					while (processedSize < payloadSize) {
						const subTransaction = txCodec.deserialize(parser);
						transaction.transactions.push({ transaction: subTransaction.entity });
						processedSize += subTransaction.size;

						const paddingSize = innerAggregateTxPaddingSize(subTransaction.size);
						if (0 < paddingSize) {
							parser.buffer(paddingSize);
							processedSize += paddingSize;
						}
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
						cosignature.version = parser.uint64();
						cosignature.signerPublicKey = parser.buffer(constants.sizes.signerPublicKey);
						cosignature.signature = parser.buffer(constants.sizes.signature);
						transaction.cosignatures.push(cosignature);
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer, txCodecs) => {
				requireCodecs(txCodecs);

				serializer.writeBuffer(transaction.transactionsHash);

				// 1. calculate payload size
				const txCodec = createSubTransactionCodec(txCodecs);

				// notice that inner tx metadata is not serialized because it is derivable
				const transactions = (transaction.transactions || []).map(transactionWithMetadata => transactionWithMetadata.transaction);
				const subTransactionSizes = [];

				let payloadSize = 0;
				transactions.forEach(subTransaction => {
					const subTransactionSize = txCodec.size(subTransaction);
					subTransactionSizes.push(subTransactionSize);
					const paddingSize = innerAggregateTxPaddingSize(subTransactionSize);
					payloadSize += subTransactionSize + paddingSize;
				});

				serializer.writeUint32(payloadSize);

				serializer.writeUint32(transaction.aggregateTransactionHeader_Reserved1);

				// 2. serialize transactions
				let i = 0;
				transactions.forEach(subTransaction => {
					const subTransactionSize = subTransactionSizes[i++];
					txCodec.serialize(subTransaction, serializer, subTransactionSize);
					const paddingSize = innerAggregateTxPaddingSize(subTransactionSize);
					if (0 < paddingSize)
						serializer.writeBuffer(Buffer.alloc(paddingSize));
				});

				// 3. serialize cosignatures
				if (transaction.cosignatures) {
					transaction.cosignatures.forEach(cosignature => {
						serializer.writeUint64(cosignature.version);
						serializer.writeBuffer(cosignature.signerPublicKey);
						serializer.writeBuffer(cosignature.signature);
					});
				}
			}
		};

		codecBuilder.addTransactionSupport(EntityType.aggregateComplete, aggregateBuilder);
		codecBuilder.addTransactionSupport(EntityType.aggregateBonded, aggregateBuilder);
	}
};

module.exports = aggregatePlugin;
