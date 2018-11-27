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

/** @module modelBinary/ModelCodecBuilder */
const blockHeaderCodec = require('./blockHeaderCodec');
const SerializedSizeCalculator = require('../serializer/SerializedSizeCalculator');
const transactionCodec = require('./transactionCodec');
const verifiableEntityCodec = require('./verifiableEntityCodec');

const constants = {
	sizes: {
		blockHeader: 192,
		transactionHeader: 120
	}
};

const isBlockType = entityType => 0 !== (0x8000 & entityType);

const findCodecs = (entityType, codecs) => {
	if (isBlockType(entityType))
		return [verifiableEntityCodec, blockHeaderCodec];

	const codec = codecs[entityType];
	if (!codec)
		throw Error(`no codec registered for '${entityType}'`);

	return [verifiableEntityCodec, transactionCodec, codec];
};

const createThrowawayConsumingCodec = size => ({
	deserialize: parser => { parser.buffer(size); }
});

const deserializeTransactions = (parser, size, txCodecs) => {
	const transactions = [];
	let remainingBytes = size;
	while (0 < remainingBytes) {
		const transactionSize = parser.uint32();
		if (transactionSize < constants.sizes.transactionHeader)
			throw Error('transaction must contain complete transaction header');

		remainingBytes -= transactionSize;
		const entity = verifiableEntityCodec.deserialize(parser);

		// allow unknown txes to be deserialized as basic txes
		const codecs = [transactionCodec];
		if (txCodecs[entity.type])
			codecs.push(txCodecs[entity.type]);
		else
			codecs.push(createThrowawayConsumingCodec(transactionSize - constants.sizes.transactionHeader));

		codecs.forEach(codec => {
			Object.assign(entity, codec.deserialize(parser, transactionSize, txCodecs));
		});

		transactions.push(entity);
	}

	return transactions;
};

/**
 * Builder for creating an aggregate model codec.
 */
class ModelCodecBuilder {
	/**
	 * Creates a model codec builder.
	 */
	constructor() {
		this.codecs = [];
	}

	/**
	 * Adds support for a typed transaction.
	 * @param {module:model/EntityType} type The transaction type.
	 * @param {object} codec The transaction codec.
	 */
	addTransactionSupport(type, codec) {
		if (isBlockType(type) || this.codecs[type])
			throw Error(`codec already registered for '${type}'`);

		this.codecs[type] = codec;
	}

	/**
	 * Builds and returns an appropriate aggregate model codec.
	 * @returns {module:modelBinary/AggregateModelCodec} The aggregate model codec.
	 */
	build() {
		const txCodecs = this.codecs;
		return {
			supports(type) {
				return isBlockType(type) || undefined !== txCodecs[type];
			},

			deserialize: (parser, options) => {
				// get codecs for the current entity (and ignore the verifiableEntity codec)
				const size = parser.uint32();
				const entity = verifiableEntityCodec.deserialize(parser);
				const codecs = findCodecs(entity.type, txCodecs);
				codecs.shift();

				codecs.forEach(codec => {
					Object.assign(entity, codec.deserialize(parser, size, txCodecs));
				});

				// if it's a block with transactions, also deserialize them
				const shouldParseBlockTransactions = !(options && options.skipBlockTransactions);
				if (shouldParseBlockTransactions && isBlockType(entity.type) && constants.sizes.blockHeader !== size) {
					const extraSize = size - constants.sizes.blockHeader;
					if (0 > extraSize)
						throw Error('block must contain complete block header');

					entity.transactions = deserializeTransactions(parser, extraSize, txCodecs);
				}

				return entity;
			},

			serialize: (entity, serializer) => {
				const codecs = findCodecs(entity.type, txCodecs);

				const sizeCalculator = new SerializedSizeCalculator();
				codecs.forEach(codec => {
					codec.serialize(entity, sizeCalculator, txCodecs);
				});

				serializer.writeUint32(sizeCalculator.size() + 4); // include size of size field itself
				codecs.forEach(codec => {
					codec.serialize(entity, serializer, txCodecs);
				});
			}
		};
	}
}

module.exports = ModelCodecBuilder;
