import { expect } from 'chai';
import ModelSchemaBuilder from '../../src/model/ModelSchemaBuilder';
import ModelType from '../../src/model/ModelType';
import EntityType from '../../src/model/EntityType';

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

				'transaction',
				'transactionMetadata',
				'transactionWithMetadata',

				'account',
				'mosaic',
				'accountMetadata',
				'accountWithMetadata',

				'chainInfo',
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

		function extractPropertiesWithType(object, matches, type, key = '') {
			const properties = Object.keys(object);
			for (const property of properties) {
				if (ModelType[type] === (object[property].type || object[property]))
					matches.push(`${key}${property}`);
				else if ('string' !== typeof object[property])
					extractPropertiesWithType(object[property], matches, type, `${key}${property}.`);
			}
		}

		function extractSchemaPropertiesWithType(type) {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const modelSchema = builder.build();

			// Act:
			const matchingProperties = [];
			extractPropertiesWithType(modelSchema, matchingProperties, type);
			return matchingProperties;
		}

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
			expect(matchingProperties.length).to.equal(6);
			expect(matchingProperties).to.deep.equal([
				'blockHeaderWithMetadata.meta',
				'blockHeaderWithMetadata.block',

				'transactionWithMetadata.meta',
				'transactionWithMetadata.transaction',

				'accountWithMetadata.meta',
				'accountWithMetadata.account'
			]);
		});

		it('exposes correct array properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('array');

			// Assert:
			expect(matchingProperties.length).to.equal(1);
			expect(matchingProperties).to.deep.equal([
				'account.mosaics'
			]);
		});

		// endregion

		// region model types

		it('exposes correct binary properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('binary');

			// Assert:
			expect(matchingProperties.length).to.equal(14);
			expect(matchingProperties).to.deep.equal([
				'verifiableEntity.signature',
				'verifiableEntity.signer',

				'blockHeader.previousBlockHash',
				'blockHeader.blockTransactionsHash',
				'blockHeader.signature',
				'blockHeader.signer',
				'blockHeaderMetadata.hash',
				'blockHeaderMetadata.generationHash',

				'transaction.signature',
				'transaction.signer',
				'transactionMetadata.hash',
				'transactionMetadata.merkleComponentHash',

				'account.address',
				'account.publicKey'
			]);
		});

		it('exposes correct uint64 properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('uint64');

			// Assert:
			expect(matchingProperties.length).to.equal(16);
			expect(matchingProperties).to.deep.equal([
				'blockHeader.height',
				'blockHeader.timestamp',
				'blockHeader.difficulty',
				'blockHeaderMetadata.totalFee',

				'transaction.deadline',
				'transaction.fee',
				'transactionMetadata.height',

				'account.addressHeight',
				'account.publicKeyHeight',
				'account.importance',
				'account.importanceHeight',
				'mosaic.id',
				'mosaic.amount',

				'chainInfo.height',
				'chainInfo.scoreLow',
				'chainInfo.scoreHigh'
			]);
		});

		it('exposes correct object id properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('objectId');

			// Assert:
			expect(matchingProperties.length).to.equal(2);
			expect(matchingProperties).to.deep.equal([
				'transactionMetadata.id',
				'transactionMetadata.aggregateId'
			]);
		});

		it('exposes correct string properties', () => {
			// Act:
			const matchingProperties = extractSchemaPropertiesWithType('string');

			// Assert:
			expect(matchingProperties.length).to.equal(0);
		});

		// endregion
	});

	describe('with extensions', () => {
		it('can add transaction extension', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport('foo', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });
			const modelSchema = builder.build();

			// Assert:
			expect(modelSchema).to.contain.key('foo');
			expect(modelSchema.foo.alpha).to.equal(ModelType.array);
			expect(modelSchema.foo.beta).to.equal(ModelType.binary);
			expect(modelSchema.foo.gamma).to.equal(ModelType.uint64);

			// - transaction extensions should inherit transaction types
			expect(modelSchema.foo.signature).to.equal(ModelType.binary);
			expect(modelSchema.foo.fee).to.equal(ModelType.uint64);
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
			expect(modelSchema.foo.fee).to.equal(undefined);
		});

		it('cannot add conflicting extensions', () => {
			// Act:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport('foo', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });
			builder.addSchema('bar', { alpha: ModelType.array, beta: ModelType.binary, gamma: ModelType.uint64 });

			// Assert:
			for (const key of ['foo', 'bar']) {
				expect(() => builder.addTransactionSupport(key, {}), key).to.throw('already registered');
				expect(() => builder.addSchema(key, {}), key).to.throw('already registered');
			}
		});

		it('cannot override default extensions', () => {
			// Act:
			const builder = new ModelSchemaBuilder();

			// Assert:
			for (const key of ['blockHeader', 'mosaic']) {
				expect(() => builder.addTransactionSupport(key, {}), key).to.throw('already registered');
				expect(() => builder.addSchema(key, {}), key).to.throw('already registered');
			}
		});

		it('picks transaction sub schema based on whitelist and availability', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			builder.addTransactionSupport('foo', { alpha: ModelType.array });
			builder.addTransactionSupport('registerNamespace', { beta: ModelType.binary });
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
			builder.addTransactionSupport('foo', { alpha: ModelType.array });
			builder.addTransactionSupport('registerNamespace', { beta: ModelType.binary });
			builder.build();

			// Act + Assert:
			expect(schemaLookup({ type: 0x8888 })).to.equal('transaction'); // not in whitelist
			expect(schemaLookup({ type: EntityType.transfer })).to.equal('transaction'); // in whitelist, not available
			expect(schemaLookup({ type: EntityType.registerNamespace })).to.equal('registerNamespace'); // in whitelist, available
		});
	});
});
