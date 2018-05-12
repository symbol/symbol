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

module.exports = transferPlugin;
