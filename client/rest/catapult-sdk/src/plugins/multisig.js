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
			addressAdditions: { type: ModelType.array, schemaName: ModelType.binary },
			addressDeletions: { type: ModelType.array, schemaName: ModelType.binary }
		});

		builder.addSchema('multisigEntry', {
			multisig: { type: ModelType.object, schemaName: 'multisigEntry.multisig' }
		});
		builder.addSchema('multisigEntry.multisig', {
			accountAddress: ModelType.binary,
			multisigAddresses: { type: ModelType.array, schemaName: ModelType.binary },
			cosignatoryAddresses: { type: ModelType.array, schemaName: ModelType.binary }
		});
		builder.addSchema('multisigGraph', {
			multisigEntries: { type: ModelType.array, schemaName: 'multisigEntry' }
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.modifyMultisigAccount, {
			deserialize: parser => {
				const transaction = {};
				transaction.minRemovalDelta = convert.uint8ToInt8(parser.uint8());
				transaction.minApprovalDelta = convert.uint8ToInt8(parser.uint8());

				const publicKeyAdditionsCount = parser.uint8();
				const publicKeyDeletionsCount = parser.uint8();

				transaction.multisigAccountModificationTransactionBody_Reserved1 = parser.uint32();

				transaction.publicKeyAdditions = [];
				for (let i = 0; i < publicKeyAdditionsCount; ++i)
					transaction.publicKeyAdditions.push(parser.buffer(constants.sizes.signerPublicKey));

				transaction.publicKeyDeletions = [];
				for (let i = 0; i < publicKeyDeletionsCount; ++i)
					transaction.publicKeyDeletions.push(parser.buffer(constants.sizes.signerPublicKey));

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(convert.int8ToUint8(transaction.minRemovalDelta));
				serializer.writeUint8(convert.int8ToUint8(transaction.minApprovalDelta));
				serializer.writeUint8(transaction.publicKeyAdditions.length);
				serializer.writeUint8(transaction.publicKeyDeletions.length);
				serializer.writeUint32(transaction.multisigAccountModificationTransactionBody_Reserved1);
				transaction.publicKeyAdditions.forEach(key => {
					serializer.writeBuffer(key);
				});
				transaction.publicKeyDeletions.forEach(key => {
					serializer.writeBuffer(key);
				});
			}
		});
	}
};

module.exports = multisigPlugin;
