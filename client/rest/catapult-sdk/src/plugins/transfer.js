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

/** @module plugins/transfer */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

/**
 * Creates a transfer plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const transferPlugin = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.transfer, {
			recipientAddress: ModelType.binary,
			message: ModelType.binary,
			mosaics: { type: ModelType.array, schemaName: 'mosaic' }
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.transfer, {
			deserialize: parser => {
				const transaction = {};
				transaction.recipientAddress = parser.buffer(constants.sizes.addressDecoded);

				const messageSize = parser.uint16();
				const numMosaics = parser.uint8();

				transaction.transferTransactionBody_Reserved1 = parser.uint32();
				transaction.transferTransactionBody_Reserved2 = parser.uint8();

				if (0 < numMosaics) {
					transaction.mosaics = [];
					while (transaction.mosaics.length < numMosaics) {
						const id = parser.uint64();
						const amount = parser.uint64();
						transaction.mosaics.push({ id, amount });
					}
				}

				if (0 < messageSize)
					transaction.message = parser.buffer(messageSize);

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeBuffer(transaction.recipientAddress);

				serializer.writeUint16(transaction.message ? transaction.message.length : 0);

				const numMosaics = transaction.mosaics ? transaction.mosaics.length : 0;
				serializer.writeUint8(numMosaics);

				serializer.writeUint32(transaction.transferTransactionBody_Reserved1);
				serializer.writeUint8(transaction.transferTransactionBody_Reserved2);

				if (0 < numMosaics) {
					transaction.mosaics.forEach(mosaic => {
						serializer.writeUint64(mosaic.id);
						serializer.writeUint64(mosaic.amount);
					});
				}

				if (transaction.message)
					serializer.writeBuffer(transaction.message);
			}
		});
	}
};

module.exports = transferPlugin;
