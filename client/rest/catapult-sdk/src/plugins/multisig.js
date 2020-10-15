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

/** @module plugins/multisig */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');
const convert = require('../utils/convert');

const constants = { sizes };

/**
 * Creates a multisig plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const multisigPlugin = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.modifyMultisigAccount, {
			minRemovalDelta: ModelType.int,
			minApprovalDelta: ModelType.int,
			addressAdditions: { type: ModelType.array, schemaName: ModelType.binary },
			addressDeletions: { type: ModelType.array, schemaName: ModelType.binary }
		});

		builder.addSchema('multisigEntry', {
			multisig: { type: ModelType.object, schemaName: 'multisigEntry.multisig' }
		});
		builder.addSchema('multisigEntry.multisig', {
			accountAddress: ModelType.binary,
			minApproval: ModelType.int,
			minRemoval: ModelType.int,
			multisigAddresses: { type: ModelType.array, schemaName: ModelType.binary },
			cosignatoryAddresses: { type: ModelType.array, schemaName: ModelType.binary }
		});
		builder.addSchema('multisigGraph', {
			level: ModelType.none,
			multisigEntries: { type: ModelType.array, schemaName: 'multisigEntry' }
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.modifyMultisigAccount, {
			deserialize: parser => {
				const transaction = {};
				transaction.minRemovalDelta = convert.uint8ToInt8(parser.uint8());
				transaction.minApprovalDelta = convert.uint8ToInt8(parser.uint8());

				const addressAdditionsCount = parser.uint8();
				const addressDeletionsCount = parser.uint8();

				transaction.multisigAccountModificationTransactionBody_Reserved1 = parser.uint32();

				transaction.addressAdditions = [];
				for (let i = 0; i < addressAdditionsCount; ++i)
					transaction.addressAdditions.push(parser.buffer(constants.sizes.addressDecoded));

				transaction.addressDeletions = [];
				for (let i = 0; i < addressDeletionsCount; ++i)
					transaction.addressDeletions.push(parser.buffer(constants.sizes.addressDecoded));

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(convert.int8ToUint8(transaction.minRemovalDelta));
				serializer.writeUint8(convert.int8ToUint8(transaction.minApprovalDelta));
				serializer.writeUint8(transaction.addressAdditions.length);
				serializer.writeUint8(transaction.addressDeletions.length);
				serializer.writeUint32(transaction.multisigAccountModificationTransactionBody_Reserved1);
				transaction.addressAdditions.forEach(key => {
					serializer.writeBuffer(key);
				});
				transaction.addressDeletions.forEach(key => {
					serializer.writeBuffer(key);
				});
			}
		});
	}
};

module.exports = multisigPlugin;
