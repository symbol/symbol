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
import ModelType from '../../../src/catapult-sdk/model/ModelType.js';
import { expect } from 'chai';
import { models } from 'symbol-sdk/symbol';

const { TransactionType } = models;

describe('model schema builder', () => {
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
				'deploymentData',
				'stateTree',
				'storageInfo',
				'storageInfoDatabase'
			]);
		});

		it('exposes no defined transaction types', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.not.contain.any.keys(Object.keys(TransactionType));
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
				'serverInfo.serverInfo',
				'serverInfoData.deployment',
				'storageInfo.database'
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
				'blockHeader.previousImportanceBlockHash',
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

				'account.publicKey',
				'accountLinkPublicKey.publicKey',
				'votingPublicKey.publicKey',

				'nodeInfo.publicKey',
				'nodeInfo.networkGenerationHashSeed',
				'nodeInfo.nodePublicKey',
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
				'blockHeader.harvestingEligibleAccountsCount',
				'blockHeader.totalVotingBalance',
				'blockHeaderMetadata.totalFee',

				'finalizedBlock.height',
				'finalizationProof.height',
				'messageGroup.height',

				'transaction.deadline',
				'transaction.maxFee',
				'transactionMetadata.height',
				'transactionMetadata.timestamp',

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
				'serverInfoData.sdkVersion',
				'deploymentData.deploymentTool',
				'deploymentData.deploymentToolVersion',
				'deploymentData.lastUpdatedDate'

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
				'storageInfo.numAccounts',
				'storageInfoDatabase.numIndexes',
				'storageInfoDatabase.numObjects',
				'storageInfoDatabase.dataSize',
				'storageInfoDatabase.indexSize',
				'storageInfoDatabase.storageSize'
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
				'blockHeader.votingEligibleAccountsCount',
				'finalizedBlock.finalizationEpoch',
				'finalizedBlock.finalizationPoint',
				'finalizationProof.version',
				'finalizationProof.finalizationEpoch',
				'finalizationProof.finalizationPoint',
				'messageGroup.stage',
				'transactionMetadata.feeMultiplier',
				'activityBucket.beneficiaryCount',
				'votingPublicKey.startEpoch',
				'votingPublicKey.endEpoch'
			]);
		});

		it('exposes correct encodedAddress properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('encodedAddress');

			// Assert:
			expect(matchingProperties).to.deep.equal([
				'blockHeader.beneficiaryAddress',
				'account.address'
			]);
		});

		// endregion
	});

	describe('with extensions', () => {
		it('can add transaction extension', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport(TransactionType.TRANSFER, {
				alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64
			});
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('TransactionType.TRANSFER');

			const transferSchema = modelSchema['TransactionType.TRANSFER'];
			expect(transferSchema.alpha).to.equal(ModelType.array);
			expect(transferSchema.beta).to.equal(ModelType.binary);
			expect(transferSchema.gamma).to.equal(ModelType.uint64);

			// - transaction extensions should inherit transaction types
			expect(transferSchema.signature).to.equal(ModelType.binary);
			expect(transferSchema.maxFee).to.equal(ModelType.uint64);
		});

		it('can add other extension', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addSchema('foo', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('foo');

			const fooSchema = modelSchema.foo;
			expect(fooSchema.alpha).to.equal(ModelType.array);
			expect(fooSchema.beta).to.equal(ModelType.binary);
			expect(fooSchema.gamma).to.equal(ModelType.uint64);

			// - non-transaction extensions should not inherit transaction types
			expect(fooSchema.signature).to.equal(undefined);
			expect(fooSchema.maxFee).to.equal(undefined);
		});

		it('can add transaction extension for known entity type', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport(TransactionType.TRANSFER, { alpha: ModelType.array });
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('TransactionType.TRANSFER');

			const transferSchema = modelSchema['TransactionType.TRANSFER'];
			expect(transferSchema.alpha).to.equal(ModelType.array);

			// - transaction extensions should inherit transaction types
			expect(transferSchema.signature).to.equal(ModelType.binary);
			expect(transferSchema.maxFee).to.equal(ModelType.uint64);
		});

		it('cannot add transaction extension for unknown entity type', () => {
			// Act + Assert:
			const builder = new ModelSchemaBuilder();
			expect(() => builder.addTransactionSupport(new TransactionType(1), { alpha: ModelType.array })).to
				.throw('invalid enum value');
		});

		it('cannot add conflicting extensions', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport(TransactionType.TRANSFER, {
				alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64
			});
			builder.addSchema('bar', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });

			// Assert:
			['TransactionType.TRANSFER', 'bar'].forEach(key => {
				expect(() => builder.addSchema(key, {}), key).to.throw('already registered');
			});
			[TransactionType.TRANSFER].forEach(type => {
				expect(() => builder.addTransactionSupport(type, {}), type).to.throw('already registered');
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

		it('picks transaction sub schema based on whitelist and availability', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport(TransactionType.NAMESPACE_REGISTRATION, { beta: ModelType.binary });
			const modelSchema = builder.build();
			const schemaLookup = modelSchema.transactionWithMetadata.transaction.schemaName;

			// Act + Assert:
			expect(schemaLookup({ type: 0x8888 })).to.equal('transaction'); // not in whitelist
			expect(schemaLookup({ type: TransactionType.TRANSFER.value })).to.equal('transaction'); // in whitelist, not available
			expect(schemaLookup({ type: TransactionType.NAMESPACE_REGISTRATION.value }))
				.to.equal('TransactionType.NAMESPACE_REGISTRATION'); // in whitelist, available
		});
	});

	describe('transaction schema name supplier', () => {
		it('picks transaction sub schema based on whitelist and availability', () => {
			// Arrange: notice that schema lookup is created BEFORE additional transactions are registered
			const builder = new ModelSchemaBuilder();
			const schemaLookup = builder.transactionSchemaNameSupplier();
			builder.addTransactionSupport(TransactionType.NAMESPACE_REGISTRATION, { beta: ModelType.binary });
			builder.build();

			// Act + Assert:
			expect(schemaLookup({ type: 0x8888 })).to.equal('transaction'); // not in whitelist
			expect(schemaLookup({ type: TransactionType.TRANSFER.value })).to.equal('transaction'); // in whitelist, not available
			expect(schemaLookup({ type: TransactionType.NAMESPACE_REGISTRATION.value }))
				.to.equal('TransactionType.NAMESPACE_REGISTRATION'); // in whitelist, available
		});
	});
});
