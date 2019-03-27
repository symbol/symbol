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

const EntityType = require('../../src/model/EntityType');
const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const ModelType = require('../../src/model/ModelType');
const { expect } = require('chai');

describe('model schema builder', () => {
	describe('allowed transaction types', () => {
		it('has entityTypes on list of allowed transactions by default', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();

			// Act + Assert:
			const typeName = builder.typeToName(EntityType.transfer);
			expect(typeName).to.equal('transfer');
		});

		it('can be altered with setAllowedTransactions', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			builder.setAllowedTransactions({});

			// Act + Assert:
			expect(() => builder.typeToName(EntityType.transfer)).to.throw('transactionType is not in the list of allowed transactions');
		});

		it('can be set to use custom types', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			builder.setAllowedTransactions({ foo: 12345 });

			// Act:
			const typeName = builder.typeToName(12345);

			// Assert:
			expect(typeName).to.equal('foo');
		});
	});

	describe('with no extensions', () => {
		it('exposes expected types', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const modelSchema = builder.build();

			// Act:
			const schemaRootProperties = Object.keys(modelSchema);

			// Assert:
			expect(schemaRootProperties).to.deep.equal([
				'verifiableEntity',

				'blockHeader',
				'blockHeaderMetadata',
				'blockHeaderWithMetadata',
				'merkleProofInfo',
				'merkleProofInfoPathNode',

				'transaction',
				'transactionMetadata',
				'transactionWithMetadata',

				'transactionStatus',

				'account',
				'mosaic',
				'accountMetadata',
				'accountWithMetadata',

				'chainInfo',
				'nodeInfo',
				'communicationTimestamps',
				'nodeTime',
				'storageInfo'
			]);
		});

		it('exposes no defined transaction types', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.not.contain.any.keys(Object.keys(EntityType));
		});

		const extractPropertiesWithType = (object, matches, type, key = '') => {
			const properties = Object.keys(object);
			properties.forEach(property => {
				if (ModelType[type] === (object[property].type || object[property]))
					matches.push(`${key}${property}`);
				else if ('string' !== typeof object[property])
					extractPropertiesWithType(object[property], matches, type, `${key}${property}.`);
			});
		};

		const extractSchemaPropertiesWithType = type => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const modelSchema = builder.build();

			// Act:
			const matchingProperties = [];
			extractPropertiesWithType(modelSchema, matchingProperties, type);
			return matchingProperties;
		};

		// region schema types

		it('exposes correct none properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('none');

			// Assert:
			expect(matchingProperties.length).to.equal(0);
		});

		it('exposes correct object properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('object');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeaderWithMetadata.meta',
				'blockHeaderWithMetadata.block',

				'transactionWithMetadata.meta',
				'transactionWithMetadata.transaction',

				'accountWithMetadata.meta',
				'accountWithMetadata.account',

				'nodeTime.communicationTimestamps'
			]);
		});

		it('exposes correct array properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('array');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeaderMetadata.subCacheMerkleRoots',
				'merkleProofInfo.merklePath',
				'account.mosaics'
			]);
		});

		// endregion

		// region model types

		it('exposes correct binary properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('binary');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'verifiableEntity.signature',
				'verifiableEntity.signer',

				'blockHeader.previousBlockHash',
				'blockHeader.blockTransactionsHash',
				'blockHeader.blockReceiptsHash',
				'blockHeader.stateHash',
				'blockHeader.beneficiary',
				'blockHeader.signature',
				'blockHeader.signer',
				'blockHeaderMetadata.hash',
				'blockHeaderMetadata.generationHash',
				'blockHeaderMetadata.subCacheMerkleRoots.schemaName',
				'merkleProofInfoPathNode.hash',

				'transaction.signature',
				'transaction.signer',
				'transactionMetadata.aggregateHash',
				'transactionMetadata.hash',
				'transactionMetadata.merkleComponentHash',

				'transactionStatus.hash',

				'account.address',
				'account.publicKey',

				'nodeInfo.publicKey'
			]);
		});

		it('exposes correct uint64 properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('uint64');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeader.height',
				'blockHeader.timestamp',
				'blockHeader.difficulty',
				'blockHeaderMetadata.totalFee',

				'transaction.deadline',
				'transaction.maxFee',
				'transactionMetadata.height',

				'transactionStatus.deadline',
				'transactionStatus.height',

				'account.addressHeight',
				'account.publicKeyHeight',
				'account.importance',
				'account.importanceHeight',
				'mosaic.id',
				'mosaic.amount',

				'chainInfo.height',
				'chainInfo.scoreLow',
				'chainInfo.scoreHigh',

				'communicationTimestamps.receiveTimestamp',
				'communicationTimestamps.sendTimestamp'
			]);
		});

		it('exposes correct object id properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('objectId');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'transactionMetadata.aggregateId',
				'transactionMetadata.id'
			]);
		});

		it('exposes correct string properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('string');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'nodeInfo.friendlyName',
				'nodeInfo.host'
			]);
		});

		it('exposes correct status code properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('statusCode');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'transactionStatus.status'
			]);
		});

		// endregion
	});

	const TestEntityType = { foo: 1234, bar: 2345 };

	describe('with extensions', () => {
		it('can add transaction extension', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.setAllowedTransactions(TestEntityType);
			builder.addTransactionSupport(TestEntityType.foo, { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('foo');
			expect(modelSchema.foo.alpha).to.equal(ModelType.array);
			expect(modelSchema.foo.beta).to.equal(ModelType.binary);
			expect(modelSchema.foo.gamma).to.equal(ModelType.uint64);

			// - transaction extensions should inherit transaction types
			expect(modelSchema.foo.signature).to.equal(ModelType.binary);
			expect(modelSchema.foo.maxFee).to.equal(ModelType.uint64);
		});

		it('can add other extension', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addSchema('foo', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('foo');
			expect(modelSchema.foo.alpha).to.equal(ModelType.array);
			expect(modelSchema.foo.beta).to.equal(ModelType.binary);
			expect(modelSchema.foo.gamma).to.equal(ModelType.uint64);

			// - non-transaction extensions should not inherit transaction types
			expect(modelSchema.foo.signature).to.equal(undefined);
			expect(modelSchema.foo.maxFee).to.equal(undefined);
		});

		it('can add transaction extension for known entity type', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport(EntityType.transfer, { alpha: ModelType.array });
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('transfer');
			expect(modelSchema.transfer.alpha).to.equal(ModelType.array);

			// - transaction extensions should inherit transaction types
			expect(modelSchema.transfer.signature).to.equal(ModelType.binary);
			expect(modelSchema.transfer.maxFee).to.equal(ModelType.uint64);
		});

		it('cannot add transaction extension for unknown entity type', () => {
			// Act + Assert:
			const builder = new ModelSchemaBuilder();
			expect(() => builder.addTransactionSupport(TestEntityType.foo, { alpha: ModelType.array })).to
				.throw(`transactionType is not in the list of allowed transactions '${TestEntityType.foo}'`);
		});

		it('cannot add conflicting extensions', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.setAllowedTransactions(TestEntityType);
			builder.addTransactionSupport(TestEntityType.foo, { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });
			builder.addSchema('bar', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });

			// Assert:
			['foo', 'bar'].forEach(key => {
				expect(() => builder.addSchema(key, {}), key).to.throw('already registered');
			});
			Object.keys(TestEntityType).forEach(key => {
				expect(() => builder.addTransactionSupport(TestEntityType[key], {}), key).to.throw('already registered');
			});
		});

		it('cannot override default extensions with schema', () => {
			// Act:
			const builder = new ModelSchemaBuilder();

			// Assert:
			['blockHeader', 'mosaic'].forEach(key => {
				expect(() => builder.addSchema(key, {}), key).to.throw('already registered');
			});
		});

		it('cannot override default extensions with transactions', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			const allowedTransactions = { blockHeader: 123, mosaic: 456 };
			builder.setAllowedTransactions(allowedTransactions);

			// Assert:
			Object.keys(allowedTransactions).forEach(key => {
				expect(() => builder.addTransactionSupport(allowedTransactions[key], {}), key).to.throw('already registered');
			});
		});

		it('picks transaction sub schema based on whitelist and availability', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			builder.setAllowedTransactions(Object.assign({}, TestEntityType, EntityType));
			builder.addTransactionSupport(TestEntityType.foo, { alpha: ModelType.array });
			builder.addTransactionSupport(EntityType.registerNamespace, { beta: ModelType.binary });
			const modelSchema = builder.build();
			const schemaLookup = modelSchema.transactionWithMetadata.transaction.schemaName;

			// Act + Assert:
			expect(schemaLookup({ type: 0x8888 })).to.equal('transaction'); // not in whitelist
			expect(schemaLookup({ type: EntityType.transfer })).to.equal('transaction'); // in whitelist, not available
			expect(schemaLookup({ type: EntityType.registerNamespace })).to.equal('registerNamespace'); // in whitelist, available
		});
	});

	describe('transaction schema name supplier', () => {
		it('picks transaction sub schema based on whitelist and availability', () => {
			// Arrange: notice that schema lookup is created BEFORE additional transactions are registered
			const builder = new ModelSchemaBuilder();
			const schemaLookup = builder.transactionSchemaNameSupplier();
			builder.setAllowedTransactions(Object.assign({}, TestEntityType, EntityType));
			builder.addTransactionSupport(TestEntityType.foo, { alpha: ModelType.array });
			builder.addTransactionSupport(EntityType.registerNamespace, { beta: ModelType.binary });
			builder.build();

			// Act + Assert:
			expect(schemaLookup({ type: 0x8888 })).to.equal('transaction'); // not in whitelist
			expect(schemaLookup({ type: EntityType.transfer })).to.equal('transaction'); // in whitelist, not available
			expect(schemaLookup({ type: EntityType.registerNamespace })).to.equal('registerNamespace'); // in whitelist, available
		});
	});
});
