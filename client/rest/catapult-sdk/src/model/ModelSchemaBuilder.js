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

/** @module model/ModelSchemaBuilder */
const EntityType = require('./EntityType');
const ModelType = require('./ModelType');

/**
 * Builder for creating a model schema.
 */
class ModelSchemaBuilder {
	/**
	 * Creates a model schema builder.
	 */
	constructor() {
		this.schema = {
			// region verifiable entity

			verifiableEntity: {
				signature: ModelType.binary,
				signerPublicKey: ModelType.binary
			},

			// endregion

			// region block

			blockHeader: {
				height: ModelType.uint64,
				timestamp: ModelType.uint64,
				difficulty: ModelType.uint64,
				previousBlockHash: ModelType.binary,
				transactionsHash: ModelType.binary,
				receiptsHash: ModelType.binary,
				stateHash: ModelType.binary,
				beneficiaryPublicKey: ModelType.binary
			},
			blockHeaderMetadata: {
				hash: ModelType.binary,
				generationHash: ModelType.binary,
				totalFee: ModelType.uint64,
				stateHashSubCacheMerkleRoots: { type: ModelType.array, schemaName: ModelType.binary }
			},
			blockHeaderWithMetadata: {
				meta: { type: ModelType.object, schemaName: 'blockHeaderMetadata' },
				block: { type: ModelType.object, schemaName: 'blockHeader' }
			},
			blockHeaderWithMetadataAndId: {
				id: ModelType.objectId,
				meta: { type: ModelType.object, schemaName: 'blockHeaderMetadata' },
				block: { type: ModelType.object, schemaName: 'blockHeader' }
			},
			merkleProofInfo: {
				merklePath: { type: ModelType.array, schemaName: 'merkleProofInfoPathNode' }
			},
			merkleProofInfoPathNode: {
				hash: ModelType.binary
			},

			// endregion

			// region transaction

			transaction: {
				deadline: ModelType.uint64,
				maxFee: ModelType.uint64
			},
			transactionMetadata: {
				aggregateHash: ModelType.binary,
				aggregateId: ModelType.objectId,
				id: ModelType.objectId,
				height: ModelType.uint64,
				hash: ModelType.binary,
				merkleComponentHash: ModelType.binary
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

			// region transactionStatus

			transactionStatus: {
				hash: ModelType.binary,
				address: ModelType.binary,
				code: ModelType.statusCode,
				deadline: ModelType.uint64,
				height: ModelType.uint64
			},

			// endregion

			// region account

			account: {
				address: ModelType.binary,
				addressHeight: ModelType.uint64,
				publicKey: ModelType.binary,
				publicKeyHeight: ModelType.uint64,
				linkedAccountKey: ModelType.binary,
				importance: ModelType.uint64,
				importanceHeight: ModelType.uint64,
				activityBuckets: { type: ModelType.array, schemaName: 'activityBucket' },
				mosaics: { type: ModelType.array, schemaName: 'mosaic' }
			},
			activityBucket: {
				startHeight: ModelType.uint64,
				totalFeesPaid: ModelType.uint64,
				rawScore: ModelType.uint64
			},
			mosaic: {
				id: ModelType.uint64HexIdentifier,
				amount: ModelType.uint64
			},
			accountMeta: {
			},
			accountWithMetadata: {
				meta: { type: ModelType.object, schemaName: 'accountMeta' },
				account: { type: ModelType.object, schemaName: 'account' }
			},

			// endregion

			// region other

			chainStatistic: {
				current: { type: ModelType.object, schemaName: 'chainStatisticCurrent' }
			},
			chainStatisticCurrent: {
				height: ModelType.uint64,
				scoreLow: ModelType.uint64,
				scoreHigh: ModelType.uint64
			},
			nodeHealth: {
			},
			nodeInfo: {
				friendlyName: ModelType.string,
				host: ModelType.string,
				publicKey: ModelType.binary
			},
			communicationTimestamps: {
				receiveTimestamp: ModelType.uint64,
				sendTimestamp: ModelType.uint64
			},
			nodeTime: {
				communicationTimestamps: { type: ModelType.object, schemaName: 'communicationTimestamps' }
			},
			serverInfo: {
			},
			stateTree: {
				tree: { type: ModelType.array, schemaName: ModelType.binary }
			},
			storageInfo: {
			}

			// endregion
		};

		Object.assign(this.schema.blockHeader, this.schema.verifiableEntity);
		Object.assign(this.schema.transaction, this.schema.verifiableEntity);

		this.setAllowedTransactions(EntityType);
	}

	/**
	 * Sets transactions allowed by addTransactionSupport.
	 * @param {object} allowedTransactions Allowed transactions.
	 */
	setAllowedTransactions(allowedTransactions) {
		// prepare reverse mapping id => string
		this.entityTypeToString = Object.keys(allowedTransactions).reduce((state, name) => {
			state[allowedTransactions[name]] = name;
			return state;
		}, {});
	}

	/**
	 * Returns name for allowed transaction.
	 * @param {module:model/EntityType} transactionType Transaction type.
	 * @returns {string} Transaction name corresponding to type.
	 */
	typeToName(transactionType) {
		if (!(transactionType in this.entityTypeToString))
			throw Error(`transactionType is not in the list of allowed transactions '${transactionType}'`);

		return this.entityTypeToString[transactionType];
	}

	/**
	 * Adds support for a transaction type.
	 * @param {module:model/EntityType} transactionType Transaction type.
	 * @param {object} schema Transaction schema.
	 */
	addTransactionSupport(transactionType, schema) {
		const name = this.typeToName(transactionType);
		this.addSchema(name, schema);
		Object.assign(this.schema[name], this.schema.transaction);
	}

	/**
	 * Adds support for a named schema.
	 * @param {string} name Schema name.
	 * @param {object} schema Schema.
	 */
	addSchema(name, schema) {
		if (this.schema[name])
			throw Error(`schema already registered for '${name}'`);

		this.schema[name] = schema;
	}

	/**
	 * Returns a function that returns the best known schema for a given transaction.
	 * @returns {function} Transaction schema lookup function.
	 */
	transactionSchemaNameSupplier() {
		// default to transaction
		return transaction => {
			const transactionName = this.entityTypeToString[transaction.type];
			return transactionName && this.schema[transactionName] ? transactionName : 'transaction';
		};
	}

	/**
	 * Builds the schema and returns an appropriate aggregate schema object.
	 * @returns {object} Aggregate schema object.
	 */
	build() {
		this.schema.transactionWithMetadata.transaction.schemaName = this.transactionSchemaNameSupplier();
		return this.schema;
	}
}

module.exports = ModelSchemaBuilder;
