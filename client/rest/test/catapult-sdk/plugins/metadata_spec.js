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
import metadataPlugin from '../../../src/catapult-sdk/plugins/metadata.js';
import { expect } from 'chai';

describe('metadata plugin', () => {
	describe('register schema', () => {
		it('adds metadata system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			metadataPlugin.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 5);
			expect(modelSchema).to.contain.all.keys([
				'accountMetadata',
				'mosaicMetadata',
				'namespaceMetadata',
				'metadata',
				'metadataEntry'
			]);

			// - accountMetadata
			expect(Object.keys(modelSchema.accountMetadata).length).to.equal(Object.keys(modelSchema.transaction).length + 5);
			expect(modelSchema.accountMetadata).to.contain.all.keys([
				'targetAddress', 'scopedMetadataKey', 'valueSizeDelta', 'valueSize', 'value'
			]);

			// - mosaicMetadata
			expect(Object.keys(modelSchema.mosaicMetadata).length).to.equal(Object.keys(modelSchema.transaction).length + 6);
			expect(modelSchema.mosaicMetadata).to.contain.all.keys([
				'targetAddress', 'scopedMetadataKey', 'targetMosaicId', 'valueSizeDelta', 'valueSize', 'value'
			]);

			// - namespaceMetadata
			expect(Object.keys(modelSchema.namespaceMetadata).length).to.equal(Object.keys(modelSchema.transaction).length + 6);
			expect(modelSchema.namespaceMetadata).to.contain.all.keys([
				'targetAddress',
				'scopedMetadataKey',
				'targetNamespaceId',
				'valueSizeDelta',
				'valueSize',
				'value'
			]);

			// - metadata
			expect(Object.keys(modelSchema.metadata).length).to.equal(2);
			expect(modelSchema.metadata).to.contain.all.keys(['metadataEntry', 'id']);

			// - metadataEntry
			expect(Object.keys(modelSchema.metadataEntry).length).to.equal(9);
			expect(modelSchema.metadataEntry).to.contain.all.keys([
				'version',
				'compositeHash',
				'sourceAddress',
				'targetAddress',
				'scopedMetadataKey',
				'targetId',
				'metadataType',
				'valueSize',
				'value'
			]);
		});
	});
});
