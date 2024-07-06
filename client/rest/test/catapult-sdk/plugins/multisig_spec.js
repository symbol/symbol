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

import ModelSchemaBuilder from '../../../src/catapult-sdk/model/ModelSchemaBuilder.js';
import multisig from '../../../src/catapult-sdk/plugins/multisig.js';
import { expect } from 'chai';

describe('multisig plugin', () => {
	describe('register schema', () => {
		it('adds multisig system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			multisig.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 4);
			expect(modelSchema).to.contain.all.keys([
				'TransactionType.MULTISIG_ACCOUNT_MODIFICATION',
				'multisigEntry',
				'multisigEntry.multisig',
				'multisigGraph'
			]);

			// - TransactionType.MULTISIG_ACCOUNT_MODIFICATION
			const multisigAccountModificationSchema = modelSchema['TransactionType.MULTISIG_ACCOUNT_MODIFICATION'];
			expect(Object.keys(multisigAccountModificationSchema).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(multisigAccountModificationSchema).to.contain.all.keys([
				'minRemovalDelta', 'minApprovalDelta', 'addressAdditions', 'addressDeletions'
			]);

			// - multisig entry
			expect(Object.keys(modelSchema.multisigEntry).length).to.equal(1);
			expect(modelSchema.multisigEntry).to.contain.all.keys(['multisig']);

			expect(Object.keys(modelSchema['multisigEntry.multisig']).length).to.equal(6);
			expect(modelSchema['multisigEntry.multisig'])
				.to.contain.all.keys(['version', 'accountAddress', 'minApproval',
					'minRemoval', 'multisigAddresses', 'cosignatoryAddresses']);

			// - multisig graph
			expect(Object.keys(modelSchema.multisigGraph).length).to.equal(2);
			expect(modelSchema.multisigGraph).to.contain.all.keys(['level', 'multisigEntries']);
		});
	});
});
