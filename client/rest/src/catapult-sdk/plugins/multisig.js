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
import ModelType from '../model/ModelType.js';
import { models } from 'symbol-sdk/symbol';

/**
 * Creates a multisig plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport(models.TransactionType.MULTISIG_ACCOUNT_MODIFICATION, {
			minRemovalDelta: ModelType.int,
			minApprovalDelta: ModelType.int,
			addressAdditions: { type: ModelType.array, schemaName: ModelType.encodedAddress },
			addressDeletions: { type: ModelType.array, schemaName: ModelType.encodedAddress }
		});

		builder.addSchema('multisigEntry', {
			multisig: { type: ModelType.object, schemaName: 'multisigEntry.multisig' }
		});
		builder.addSchema('multisigEntry.multisig', {
			version: ModelType.uint16,
			accountAddress: ModelType.encodedAddress,
			minApproval: ModelType.int,
			minRemoval: ModelType.int,
			multisigAddresses: { type: ModelType.array, schemaName: ModelType.encodedAddress },
			cosignatoryAddresses: { type: ModelType.array, schemaName: ModelType.encodedAddress }
		});
		builder.addSchema('multisigGraph', {
			level: ModelType.none,
			multisigEntries: { type: ModelType.array, schemaName: 'multisigEntry' }
		});
	}
};
