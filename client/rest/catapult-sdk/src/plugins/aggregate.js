/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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
const embeddedEntityCodec = require('../modelBinary/embeddedEntityCodec');
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const SerializedSizeCalculator = require('../serializer/SerializedSizeCalculator');
const sizes = require('../modelBinary/sizes');

const constants = { sizes: {} };
Object.assign(constants.sizes, sizes, {
	aggregate: 120 + 4, // size passed into deserialize includes full transaction size (even previously processed parts)
	embedded: 40,
	cosignature: sizes.signer + sizes.signature
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

/**
 * Creates an aggregate plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const aggregatePlugin = {
	registerSchema: builder => {
		const aggregateSchema = {
			transactions: { type: ModelType.array, schemaName: 'transactionWithMetadata' },
			cosignatures: { type: ModelType.array, schemaName: 'aggregate.cosignature' }
		};

		builder.addTransactionSupport(EntityType.aggregateComplete, aggregateSchema);
		builder.addTransactionSupport(EntityType.aggregateBonded, aggregateSchema);

		builder.addSchema('aggregate.cosignature', {
			signer: ModelType.binary,
			signature: ModelType.binary,
			parentHash: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		const aggregateBuilder = {
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
						transaction.transactions.push({ transaction: subTransaction.entity });
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

				// notice that inner tx metadata is not serialized because it is derivable
				const transactions = (transaction.transactions || []).map(transactionWithMetadata => transactionWithMetadata.transaction);
				const subTransactionSizes = [];

				let payloadSize = 0;
				transactions.forEach(subTransaction => {
					const subTransactionSize = txCodec.size(subTransaction);
					subTransactionSizes.push(subTransactionSize);
					payloadSize += subTransactionSize;
				});

				serializer.writeUint32(payloadSize);

				// 2. serialize transactions
				let i = 0;
				transactions.forEach(subTransaction => {
					txCodec.serialize(subTransaction, serializer, subTransactionSizes[i++]);
				});

				// 3. serialize cosignatures
				if (transaction.cosignatures) {
					transaction.cosignatures.forEach(cosignature => {
						serializer.writeBuffer(cosignature.signer);
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
