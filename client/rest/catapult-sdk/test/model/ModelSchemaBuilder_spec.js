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
				'finalizedBlock',
				'finalizationProof',
				'messageGroup',
				'bmTreeSignature',
				'parentPublicKeySignaturePair',

				'transaction',
				'transactionMetadata',
				'transactionWithMetadata',

				'transactionStatus',

				'accountWithMetadata',
				'account',
				'supplementalPublicKey',
				'activityBucket',
				'mosaic',
				'accountLinkPublicKey',
				'accountLinkPublicKey.voting',
				'votingPublicKey',

				'chainInfo',
				'nodeHealth',
				'nodeHealthStatus',
				'nodeInfo',
				'communicationTimestamps',
				'nodeTime',
				'serverInfo',
				'serverInfoData',
				'stateTree',
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

		const extractPropertiesWithType = (object, matches, propertyType, key = '') => {
			const getTypeIfNotBasicType = obj => {
				const objKeys = Object.keys(obj);
				return 2 === objKeys.length && objKeys.includes('type') && objKeys.includes('schemaName') ? obj.type : undefined;
			};

			Object.keys(object).forEach(property => {
				if (ModelType[propertyType] === (getTypeIfNotBasicType(object[property]) || object[property]))
					matches.push(`${key}${property}`);
				else if ('string' !== typeof object[property])
					extractPropertiesWithType(object[property], matches, propertyType, `${key}${property}.`);
			});
		};

		const extractSchemaPropertiesWithType = propertyType => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const modelSchema = builder.build();

			// Act:
			const matchingProperties = [];
			extractPropertiesWithType(modelSchema, matchingProperties, propertyType);
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

				'bmTreeSignature.root',
				'bmTreeSignature.top',
				'bmTreeSignature.bottom',

				'transactionWithMetadata.meta',
				'transactionWithMetadata.transaction',

				'accountWithMetadata.account',
				'account.supplementalPublicKeys',
				'supplementalPublicKey.linked',
				'supplementalPublicKey.node',
				'supplementalPublicKey.vrf',
				'supplementalPublicKey.voting',

				'chainInfo.latestFinalizedBlock',

				'nodeHealth.status',
				'nodeTime.communicationTimestamps',
				'serverInfo.serverInfo'
			]);
		});

		it('exposes correct array properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('array');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeaderMetadata.stateHashSubCacheMerkleRoots',
				'merkleProofInfo.merklePath',
				'finalizationProof.messageGroups',
				'messageGroup.hashes',
				'messageGroup.signatures',
				'account.activityBuckets',
				'account.mosaics',
				'accountLinkPublicKey.voting.publicKeys',
				'stateTree.tree'
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
				'verifiableEntity.signerPublicKey',

				'blockHeader.proofGamma',
				'blockHeader.proofVerificationHash',
				'blockHeader.proofScalar',
				'blockHeader.previousBlockHash',
				'blockHeader.transactionsHash',
				'blockHeader.receiptsHash',
				'blockHeader.stateHash',
				'blockHeader.beneficiaryAddress',
				'blockHeader.signature',
				'blockHeader.signerPublicKey',
				'blockHeaderMetadata.hash',
				'blockHeaderMetadata.generationHash',
				'blockHeaderMetadata.stateHashSubCacheMerkleRoots.schemaName',
				'merkleProofInfoPathNode.hash',
				'finalizedBlock.hash',
				'finalizationProof.hash',
				'messageGroup.hashes.schemaName',
				'parentPublicKeySignaturePair.parentPublicKey',
				'parentPublicKeySignaturePair.signature',

				'transaction.signature',
				'transaction.signerPublicKey',
				'transactionMetadata.aggregateHash',
				'transactionMetadata.hash',
				'transactionMetadata.merkleComponentHash',

				'transactionStatus.hash',

				'account.address',
				'account.publicKey',
				'accountLinkPublicKey.publicKey',
				'votingPublicKey.publicKey',

				'nodeInfo.publicKey',
				'nodeInfo.networkGenerationHashSeed',
				'stateTree.tree.schemaName'
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
				'finalizedBlock.height',
				'finalizationProof.height',
				'messageGroup.height',

				'transaction.deadline',
				'transaction.maxFee',
				'transactionMetadata.height',

				'transactionStatus.deadline',
				'transactionStatus.height',

				'account.addressHeight',
				'account.publicKeyHeight',
				'account.importance',
				'account.importanceHeight',
				'activityBucket.startHeight',
				'activityBucket.totalFeesPaid',
				'activityBucket.rawScore',
				'mosaic.amount',

				'chainInfo.height',
				'chainInfo.scoreLow',
				'chainInfo.scoreHigh',

				'communicationTimestamps.receiveTimestamp',
				'communicationTimestamps.sendTimestamp'
			]);
		});

		it('exposes correct uint64HexIdentifier properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('uint64HexIdentifier');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'mosaic.id'
			]);
		});

		it('exposes correct object id properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('objectId');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeaderWithMetadata.id',
				'transactionMetadata.aggregateId',
				'transactionWithMetadata.id',
				'accountWithMetadata.id'
			]);
		});

		it('exposes correct string properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('string');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'merkleProofInfoPathNode.position',
				'transactionStatus.group',
				'nodeHealthStatus.apiNode',
				'nodeHealthStatus.db',
				'nodeInfo.friendlyName',
				'nodeInfo.host',
				'serverInfoData.restVersion',
				'serverInfoData.sdkVersion'
			]);
		});

		it('exposes correct status code properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('statusCode');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'transactionStatus.code'
			]);
		});

		it('exposes correct int properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('int');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeader.size',
				'blockHeader.type',
				'blockHeaderMetadata.totalTransactionsCount',
				'blockHeaderMetadata.transactionsCount',
				'blockHeaderMetadata.statementsCount',

				'transaction.size',
				'transaction.type',
				'transactionMetadata.index',

				'nodeInfo.roles',
				'nodeInfo.port',
				'nodeInfo.networkIdentifier',

				'storageInfo.numBlocks',
				'storageInfo.numTransactions',
				'storageInfo.numAccounts'
			]);
		});

		it('exposes correct uint8 properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('uint8');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeader.version',
				'blockHeader.network',
				'transaction.version',
				'transaction.network',
				'account.accountType',
				'nodeInfo.version'
			]);
		});

		it('exposes correct uint32 properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('uint32');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeader.feeMultiplier',
				'finalizedBlock.finalizationEpoch',
				'finalizedBlock.finalizationPoint',
				'finalizationProof.version',
				'finalizationProof.finalizationEpoch',
				'finalizationProof.finalizationPoint',
				'messageGroup.stage',
				'activityBucket.beneficiaryCount',
				'votingPublicKey.startEpoch',
				'votingPublicKey.endEpoch'
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
