import { expect } from 'chai';
import ModelFormatterBuilder from '../../src/model/ModelFormatterBuilder';
import ModelSchemaBuilder from '../../src/model/ModelSchemaBuilder';
import ModelType from '../../src/model/ModelType';

const modelSchema = new ModelSchemaBuilder().build();
const formattingRules = {
	[ModelType.none]: () => 'none',
	[ModelType.binary]: () => 'binary',
	[ModelType.uint64]: () => 'uint64',
	[ModelType.objectId]: () => 'objectId',
	[ModelType.string]: () => 'string'
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
				'transactionWithMetadata',
				'blockHeaderWithMetadata',
				'accountWithMetadata',
				'chainInfo',
				'storageInfo'
			]);
		});

		it('can format transaction with metadata', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.transactionWithMetadata.format({
				meta: {
					id: 0,
					height: 0,
					hash: 0
				},
				transaction: {
					signature: 0,
					signer: 0,
					version: 0,
					type: 0,

					fee: 0,
					deadline: 0
				}
			});

			// Assert:
			expect(result).to.deep.equal({
				meta: {
					id: 'objectId',
					height: 'uint64',
					hash: 'binary'
				},
				transaction: {
					signature: 'binary',
					signer: 'binary',
					version: 'none',
					type: 'none',

					fee: 'uint64',
					deadline: 'uint64'
				}
			});
		});

		it('can format block header with metadata', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.blockHeaderWithMetadata.format({
				meta: {
					hash: 0,
					generationHash: 0,
					totalFee: 0,
					numTransactions: 0
				},
				block: {
					signature: 0,
					signer: 0,
					version: 0,
					type: 0,

					height: 0,
					timestamp: 0,
					difficulty: 0,
					previousBlockHash: 0,
					blockTransactionsHash: 0
				}
			});

			// Assert:
			expect(result).to.deep.equal({
				meta: {
					hash: 'binary',
					generationHash: 'binary',
					totalFee: 'uint64',
					numTransactions: 'none'
				},
				block: {
					signature: 'binary',
					signer: 'binary',
					version: 'none',
					type: 'none',

					height: 'uint64',
					timestamp: 'uint64',
					difficulty: 'uint64',
					previousBlockHash: 'binary',
					blockTransactionsHash: 'binary'
				}
			});
		});

		it('can format account with metadata', () => {
			// Arrange:
			const formatter = new ModelFormatterBuilder().build(modelSchema, formattingRules);

			// Act:
			const result = formatter.accountWithMetadata.format({
				meta: {
				},
				account: {
					address: 0,
					addressHeight: 0,
					publicKey: 0,
					publicKeyHeight: 0,
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
				meta: {
				},
				account: {
					address: 'binary',
					addressHeight: 'uint64',
					publicKey: 'binary',
					publicKeyHeight: 'uint64',
					importance: 'uint64',
					importanceHeight: 'uint64',
					mosaics: [
						{ id: 'uint64', amount: 'uint64' },
						{ id: 'uint64', amount: 'uint64' }
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
				scoreHigh: 0
			});

			// Assert:
			expect(result).to.deep.equal({
				height: 'uint64',
				scoreLow: 'uint64',
				scoreHigh: 'uint64'
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
				numBlocks: 'none',
				numTransactions: 'none',
				numAccounts: 'none'
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
			expect(subFormatterTypes).to.deep.equal({ id: 'uint64', amount: 'uint64' });
		});
	});
});
