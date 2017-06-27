/** @module model/ModelSchemaBuilder */
import EntityType from './EntityType';
import ModelType from './ModelType';

/**
 * Builder for creating a model schema.
 */
export default class ModelSchemaBuilder {
	/**
	 * Creates a model schema builder.
	 */
	constructor() {
		this.schema = {
			// region verifiable entity

			verifiableEntity: {
				signature: ModelType.binary,
				signer: ModelType.binary
			},

			// endregion

			// region block

			blockHeader: {
				height: ModelType.uint64,
				timestamp: ModelType.uint64,
				difficulty: ModelType.uint64,
				previousBlockHash: ModelType.binary,
				blockTransactionsHash: ModelType.binary
			},
			blockHeaderMetadata: {
				hash: ModelType.binary,
				generationHash: ModelType.binary,
				totalFee: ModelType.uint64
			},
			blockHeaderWithMetadata: {
				meta: { type: ModelType.object, schemaName: 'blockHeaderMetadata' },
				block: { type: ModelType.object, schemaName: 'blockHeader' }
			},

			// endregion

			// region transaction

			transaction: {
				deadline: ModelType.uint64,
				fee: ModelType.uint64
			},
			transactionMetadata: {
				id: ModelType.objectId,
				height: ModelType.uint64,
				hash: ModelType.binary,
				merkleComponentHash: ModelType.binary,
				aggregateId: ModelType.objectId
			},
			transactionWithMetadata: {
				meta: { type: ModelType.object, schemaName: 'transactionMetadata' },
				transaction: {
					type: ModelType.object,
					// notice that this needs to be set in build to allow graceful fallback when some txes are not registered
					schemaName: undefined
				}
			},

			// endregion

			// region account

			account: {
				address: ModelType.binary,
				addressHeight: ModelType.uint64,
				publicKey: ModelType.binary,
				publicKeyHeight: ModelType.uint64,
				importance: ModelType.uint64,
				importanceHeight: ModelType.uint64,
				mosaics: { type: ModelType.array, schemaName: 'mosaic' }
			},
			mosaic: {
				id: ModelType.uint64,
				amount: ModelType.uint64
			},
			accountMetadata: {
			},
			accountWithMetadata: {
				meta: { type: ModelType.object, schemaName: 'accountMetadata' },
				account: { type: ModelType.object, schemaName: 'account' }
			},

			// endregion

			// region other

			chainInfo: {
				height: ModelType.uint64,
				scoreLow: ModelType.uint64,
				scoreHigh: ModelType.uint64
			},

			storageInfo: {
			}

			// endregion
		};

		Object.assign(this.schema.blockHeader, this.schema.verifiableEntity);
		Object.assign(this.schema.transaction, this.schema.verifiableEntity);
	}

	/**
	 * Adds support for a named transaction.
	 * @param {string} name The transaction name.
	 * @param {object} schema The transaction schema.
	 */
	addTransactionSupport(name, schema) {
		this.addSchema(name, schema);
		Object.assign(this.schema[name], this.schema.transaction);
	}

	/**
	 * Adds support for a named schema.
	 * @param {string} name The schema name.
	 * @param {object} schema The schema.
	 */
	addSchema(name, schema) {
		if (this.schema[name])
			throw Error(`schema already registered for '${name}'`);

		this.schema[name] = schema;
	}

	/**
	 * Returns a function that returns the best known schema for a given transaction.
	 * @returns {function} The transaction schema lookup function.
	 */
	transactionSchemaNameSupplier() {
		// make reverse mapping id => string
		const entityTypeToString = Object.keys(EntityType).reduce((state, name) => {
			state[EntityType[name]] = name;
			return state;
		}, {});

		// default to transaction
		return transaction => {
			const transactionName = entityTypeToString[transaction.type];
			return transactionName && this.schema[transactionName] ? transactionName : 'transaction';
		};
	}

	/**
	 * Builds the schema and returns an appropriate aggregate schema object.
	 * @returns {object} The aggregate schema object.
	 */
	build() {
		this.schema.transactionWithMetadata.transaction.schemaName = this.transactionSchemaNameSupplier();
		return this.schema;
	}
}
