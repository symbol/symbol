/** @module plugins/lock */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes: {} };
Object.assign(constants.sizes, sizes, {
	hash512: 64
});

const commonLockInfoSchema = {
	account: ModelType.binary,
	accountAddress: ModelType.binary,
	mosaicId: ModelType.uint64,
	amount: ModelType.uint64,
	height: ModelType.uint64
	/* status */
};

const commonLockTransactionSchema = {
	mosaicId: ModelType.uint64,
	amount: ModelType.uint64,
	duration: ModelType.uint64
};

const parseLockData = (parser, transaction) => {
	transaction.mosaicId = parser.uint64();
	transaction.amount = parser.uint64();
	transaction.duration = parser.uint64();
};

const serializeLockData = (transaction, serializer) => {
	serializer.writeUint64(transaction.mosaicId);
	serializer.writeUint64(transaction.amount);
	serializer.writeUint64(transaction.duration);
};

/**
 * Creates a lock plugin.
 * @type {module:plugins/CatapultPlugin}
 */
module.exports = {
	registerSchema: builder => {
		builder.addSchema('hashLockInfo', {
			lock: { type: ModelType.object, schemaName: 'hashLockInfo.lock' }
		});
		builder.addSchema('hashLockInfo.lock', Object.assign({}, commonLockInfoSchema, {
			hash: ModelType.binary
		}));

		builder.addSchema('secretLockInfo', {
			lock: { type: ModelType.object, schemaName: 'secretLockInfo.lock' }
		});
		builder.addSchema('secretLockInfo.lock', Object.assign({}, commonLockInfoSchema, {
			/* hashAlgorithm */
			secret: ModelType.binary,
			recipient: ModelType.binary
		}));

		builder.addTransactionSupport(EntityType.hashLock, Object.assign({}, commonLockTransactionSchema, {
			hash: ModelType.binary
		}));
		builder.addTransactionSupport(EntityType.secretLock, Object.assign({}, commonLockTransactionSchema, {
			secret: ModelType.binary,
			recipient: ModelType.binary
		}));
		builder.addTransactionSupport(EntityType.secretProof, {
			secret: ModelType.binary,
			proof: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.hashLock, {
			deserialize: parser => {
				const transaction = {};
				parseLockData(parser, transaction);

				transaction.hash = parser.buffer(constants.sizes.hash);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializeLockData(transaction, serializer);

				serializer.writeBuffer(transaction.hash);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.secretLock, {
			deserialize: parser => {
				const transaction = {};
				parseLockData(parser, transaction);

				transaction.hashAlgorithm = parser.uint8();
				transaction.secret = parser.buffer(constants.sizes.hash512);
				transaction.recipient = parser.buffer(constants.sizes.addressDecoded);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializeLockData(transaction, serializer);

				serializer.writeUint8(transaction.hashAlgorithm);
				serializer.writeBuffer(transaction.secret);
				serializer.writeBuffer(transaction.recipient);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.secretProof, {
			deserialize: parser => {
				const transaction = {};
				transaction.hashAlgorithm = parser.uint8();
				transaction.secret = parser.buffer(constants.sizes.hash512);
				const proofSize = parser.uint16();
				transaction.proof = parser.buffer(proofSize);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.hashAlgorithm);
				serializer.writeBuffer(transaction.secret);
				const proofSize = transaction.proof.length;
				serializer.writeUint16(proofSize);
				serializer.writeBuffer(transaction.proof);
			}
		});
	}
};
