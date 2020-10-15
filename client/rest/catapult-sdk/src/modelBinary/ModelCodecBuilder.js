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

/** @module modelBinary/ModelCodecBuilder */
const blockHeaderCodec = require('./blockHeaderCodec');
const transactionCodec = require('./transactionCodec');
const verifiableEntityCodec = require('./verifiableEntityCodec');
const SerializedSizeCalculator = require('../serializer/SerializedSizeCalculator');

const isBlockType = entityType => 0 !== (0x8000 & entityType);

const findCodecs = (entityType, codecs) => {
	if (isBlockType(entityType))
		return [verifiableEntityCodec, blockHeaderCodec];

	const codec = codecs[entityType];
	if (!codec)
		throw Error(`no codec registered for '${entityType}'`);

	return [verifiableEntityCodec, transactionCodec, codec];
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
	 * @param {module:model/EntityType} type Transaction type.
	 * @param {object} codec Transaction codec.
	 */
	addTransactionSupport(type, codec) {
		if (isBlockType(type) || this.codecs[type])
			throw Error(`codec already registered for '${type}'`);

		this.codecs[type] = codec;
	}

	/**
	 * Builds and returns an appropriate aggregate model codec.
	 * @returns {module:modelBinary/AggregateModelCodec} Aggregate model codec.
	 */
	build() {
		const txCodecs = this.codecs;
		return {
			supports(type) {
				return isBlockType(type) || undefined !== txCodecs[type];
			},

			deserialize: parser => {
				// get codecs for the current entity (and ignore the verifiableEntity codec)
				const size = parser.uint32();
				const entity = verifiableEntityCodec.deserialize(parser);
				const codecs = findCodecs(entity.type, txCodecs);
				codecs.shift();

				codecs.forEach(codec => {
					Object.assign(entity, codec.deserialize(parser, size, txCodecs));
				});

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
