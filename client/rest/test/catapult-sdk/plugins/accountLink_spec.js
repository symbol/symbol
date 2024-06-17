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
import accountLinkPlugin from '../../../src/catapult-sdk/plugins/accountLink.js';
import { expect } from 'chai';

describe('account link plugin', () => {
	describe('register schema', () => {
		it('adds account link system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			accountLinkPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 4);
			expect(modelSchema).to.contain.all.keys(['accountLink', 'nodeKeyLink', 'votingKeyLink', 'vrfKeyLink']);

			// - accountLink
			expect(Object.keys(modelSchema.accountLink).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.accountLink).to.contain.all.keys(['linkedPublicKey', 'linkAction']);

			// - nodeKeyLink
			expect(Object.keys(modelSchema.nodeKeyLink).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.nodeKeyLink).to.contain.all.keys(['linkedPublicKey', 'linkAction']);

			// - votingKeyLink
			expect(Object.keys(modelSchema.votingKeyLink).length).to.equal(Object.keys(modelSchema.transaction).length + 4);
			expect(modelSchema.votingKeyLink).to.contain.all.keys(['linkedPublicKey', 'startEpoch', 'endEpoch', 'linkAction']);

			// - vrfKeyLink
			expect(Object.keys(modelSchema.vrfKeyLink).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.vrfKeyLink).to.contain.all.keys(['linkedPublicKey', 'linkAction']);
		});
	});
});
