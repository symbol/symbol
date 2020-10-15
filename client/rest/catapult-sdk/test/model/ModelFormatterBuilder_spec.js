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

const ModelFormatterBuilder = require('../../src/model/ModelFormatterBuilder');
const ModelSchemaBuilder = require('../../src/model/ModelSchemaBuilder');
const ModelType = require('../../src/model/ModelType');
const { expect } = require('chai');

const modelSchema = new ModelSchemaBuilder().build();
const formattingRules = {
	[ModelType.none]: () => 'none',
	[ModelType.binary]: () => 'binary',
	[ModelType.uint8]: () => 'uint8',
	[ModelType.uint16]: () => 'uint16',
	[ModelType.uint32]: () => 'uint32',
	[ModelType.uint64]: () => 'uint64',
	[ModelType.uint64HexIdentifier]: () => 'uint64HexIdentifier',
	[ModelType.objectId]: () => 'objectId',
	[ModelType.string]: () => 'string',
	[ModelType.int]: () => 'int'
};

describe('model formatter builder', () => {
	describe('for built in formatter', () => {
		it('can create default formatter', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const subFormatterTypes = Object.keys(formatter);

			// Assert:
			expect(subFormatterTypes).to.deep.equal([
				'accountWithMetadata',
				'blockHeaderWithMetadata',
				'transactionWithMetadata',

				'chainInfo',
				'merkleProofInfo',
				'finalizedBlock',
				'finalizationProof',
				'nodeHealth',
				'nodeInfo',
				'nodeTime',
				'serverInfo',
				'stateTree',
				'storageInfo',
				'transactionStatus'
			]);
		});

		it('can format transaction with metadata', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.transactionWithMetadata.format({
				meta: {
					height: 0,
					hash: 0,
					index: 0
				},
				transaction: {
					signature: 0,
					signerPublicKey: 0,
					version: 0,
					network: 0,
					type: 0,

					maxFee: 0,
					deadline: 0
				}
			});

			// Assert:
			expect(result).to.deep.equal({
				meta: {
					height: 'uint64',
					hash: 'binary',
					index: 'int'
				},
				transaction: {
					signature: 'binary',
					signerPublicKey: 'binary',
					version: 'uint8',
					network: 'uint8',
					type: 'int',

					maxFee: 'uint64',
					deadline: 'uint64'
				}
			});
		});

		it('can format block header with metadata', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.blockHeaderWithMetadata.format({
				id: 0x5E3CD1498E18164DD5536133,
				meta: {
					hash: 0,
					generationHash: 0,
					totalFee: 0,
					totalTransactionsCount: 0,
					transactionsCount: 0,
					statementsCount: 0,
					stateHashSubCacheMerkleRoots: [0]
				},
				block: {
					signature: 0,
					signerPublicKey: 0,
					version: 0,
					network: 0,
					type: 0,

					height: 0,
					timestamp: 0,
					difficulty: 0,
					previousBlockHash: 0,
					transactionsHash: 0,
					receiptsHash: 0,
					stateHash: 0,
					beneficiaryAddress: 0
				}
			});

			// Assert:
			expect(result).to.deep.equal({
				id: 'objectId',
				meta: {
					hash: 'binary',
					generationHash: 'binary',
					totalFee: 'uint64',
					transactionsCount: 'int',
					statementsCount: 'int',
					totalTransactionsCount: 'int',

					stateHashSubCacheMerkleRoots: ['binary']
				},
				block: {
					signature: 'binary',
					signerPublicKey: 'binary',
					version: 'uint8',
					network: 'uint8',
					type: 'int',

					height: 'uint64',
					timestamp: 'uint64',
					difficulty: 'uint64',
					previousBlockHash: 'binary',
					transactionsHash: 'binary',
					receiptsHash: 'binary',
					stateHash: 'binary',
					beneficiaryAddress: 'binary'
				}
			});
		});

		it('can format account with metadata', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.accountWithMetadata.format({
				id: 0,
				account: {
					address: 0,
					addressHeight: 0,
					publicKey: 0,
					publicKeyHeight: 0,
					accountType: 0,
					importance: 0,
					importanceHeight: 0,
					mosaics: [
						{ id: 0, amount: 0 },
						{ id: 0, amount: 0 }
					]
				}
			});

			// Assert:
			expect(result).to.deep.equal({
				id: 'objectId',
				account: {
					address: 'binary',
					addressHeight: 'uint64',
					publicKey: 'binary',
					publicKeyHeight: 'uint64',
					accountType: 'uint8',
					importance: 'uint64',
					importanceHeight: 'uint64',
					mosaics: [
						{ id: 'uint64HexIdentifier', amount: 'uint64' },
						{ id: 'uint64HexIdentifier', amount: 'uint64' }
					]
				}
			});
		});

		it('can format chain info', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.chainInfo.format({
				height: 0,
				scoreLow: 0,
				scoreHigh: 0,
				latestFinalizedBlock: {
					height: 0,
					hash: 0,
					finalizationEpoch: 0,
					finalizationPoint: 0
				}
			});

			// Assert:
			expect(result).to.deep.equal({
				height: 'uint64',
				scoreLow: 'uint64',
				scoreHigh: 'uint64',
				latestFinalizedBlock: {
					height: 'uint64',
					hash: 'binary',
					finalizationEpoch: 'uint32',
					finalizationPoint: 'uint32'
				}
			});
		});

		it('can format storage info', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.storageInfo.format({
				numBlocks: 0,
				numTransactions: 0,
				numAccounts: 0
			});

			// Assert:
			expect(result).to.deep.equal({
				numBlocks: 'int',
				numTransactions: 'int',
				numAccounts: 'int'
			});
		});
	});

	describe('for custom formatter', () => {
		it('can add arbitrary formatter', () => {
			// Arrange:
			const builder = new ModelFormatterBuilder();
			builder.addFormatter('mosaic');
			const formatter = builder.build(modelSchema, formattingRules);

			// Act:
			const subFormatterTypes = formatter.mosaic.format({ id: 0, amount: 0 });

			// Assert:
			expect(subFormatterTypes).to.deep.equal({ id: 'uint64HexIdentifier', amount: 'uint64' });
		});

		it('cannot add arbitrary formatter multiple times', () => {
			// Arrange:
			const builder = new ModelFormatterBuilder();
			builder.addFormatter('mosaic');

			// Act + Assert:
			expect(() => { builder.addFormatter('mosaic'); }).to.throw('formatter already registered');
		});
	});
});
