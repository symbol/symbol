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
import mosaic from '../../../src/catapult-sdk/plugins/mosaic.js';
import { expect } from 'chai';

describe('mosaic plugin', () => {
	describe('register schema', () => {
		it('adds mosaic system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			mosaic.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 5);
			expect(modelSchema).to.contain.all.keys(
				'mosaicDefinition',
				'mosaicSupplyChange',
				'mosaicSupplyRevocation',
				'mosaicDescriptor',
				'mosaicDescriptor.mosaic'
			);

			// - mosaic definition
			expect(Object.keys(modelSchema.mosaicDefinition).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.mosaicDefinition).to.contain.all.keys(['id', 'duration', 'nonce', 'flags', 'divisibility']);

			// - mosaic supply change
			expect(Object.keys(modelSchema.mosaicSupplyChange).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema.mosaicSupplyChange).to.contain.all.keys(['mosaicId', 'delta', 'action']);

			// - mosaic descriptor
			expect(Object.keys(modelSchema.mosaicDescriptor).length).to.equal(2);
			expect(modelSchema.mosaicDescriptor).to.contain.all.keys(['id', 'mosaic']);

			// - mosaic descriptor mosaic
			expect(Object.keys(modelSchema['mosaicDescriptor.mosaic']).length).to.equal(9);
			expect(modelSchema['mosaicDescriptor.mosaic']).to.contain.all.keys([
				'version', 'id', 'supply', 'startHeight', 'ownerAddress', 'revision', 'flags', 'divisibility', 'duration'
			]);
		});
	});
});
