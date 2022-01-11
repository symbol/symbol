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

const EntityType = require('../../../src/catapult-sdk/model/EntityType');
const ModelSchemaBuilder = require('../../../src/catapult-sdk/model/ModelSchemaBuilder');
const ModelType = require('../../../src/catapult-sdk/model/ModelType');
const BinaryParser = require('../../../src/catapult-sdk/parser/BinaryParser');
const aggregate = require('../../../src/catapult-sdk/plugins/aggregate');
const BinarySerializer = require('../../../src/catapult-sdk/serializer/BinarySerializer');
const uint64 = require('../../../src/catapult-sdk/utils/uint64');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const constants = {
	knownTxType: 0x0022,
	sizes: {
		aggregate: 128 + 32 + 4 + 4, // transaction header + transactionshash + payload size + aggregateTransactionHeader_Reserved1
		transaction: 128,
		embedded: 48 + 8,
		cosignature: 8 + 32 + 64, // version + signer public key + signature
		transactionsHash: 32
	}
};

describe('aggregate plugin', () => {
	describe('register schema', () => {
		const assertAddsSchema = schemaName => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			aggregate.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 3);
			expect(modelSchema).to.contain.all.keys(['aggregateComplete', 'aggregateBonded', 'aggregate.cosignature']);

			// - aggregate
			expect(Object.keys(modelSchema[schemaName]).length).to.equal(Object.keys(modelSchema.transaction).length + 3);
			expect(modelSchema[schemaName]).to.contain.all.keys(['transactionsHash', 'transactions', 'cosignatures']);

			// - cosignature
			expect(modelSchema['aggregate.cosignature']).to.deep.equal({
				version: ModelType.uint64,
				signerPublicKey: ModelType.binary,
				signature: ModelType.binary,
				parentHash: ModelType.binary
			});
		};

		it('adds aggregateComplete system schema', () => assertAddsSchema('aggregateComplete'));
		it('adds aggregateBonded system schema', () => assertAddsSchema('aggregateBonded'));
	});

	describe('register codecs', () => {
		const getCodecs = () => {
			const codecs = {};
			aggregate.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		};

		it('adds aggregate codec', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codec was registered
			expect(Object.keys(codecs).length).to.equal(2);
			expect(codecs).to.contain.all.keys([EntityType.aggregateComplete.toString(), EntityType.aggregateBonded.toString()]);
		});

		const getSubTxCodecs = () => {
			const txCodecs = [];
			// notice that this codec (unlike the one in ModelCodecBuilder_spec assumes that it is embedded)
			txCodecs[constants.knownTxType] = {
				deserialize: parser => {
					const transaction = {};
					transaction.alpha = parser.uint32();
					transaction.beta = parser.uint32();

					// use of extraSize allows tests with variably sized transactions
					const extraSize = transaction.beta & 0xFF;
					if (0 < extraSize)
						parser.buffer(extraSize);

					return transaction;
				},

				serialize: (transaction, serializer) => {
					serializer.writeUint32(transaction.alpha);
					serializer.writeUint32(transaction.beta);

					const extraSize = transaction.beta & 0xFF;
					serializer.writeBuffer(Buffer.alloc(extraSize));
				}
			};

			return txCodecs;
		};

		const generateAggregate = () => {
			const transactionsHash = Buffer.concat([
				Buffer.of(0x02, 0x04, 0x80, 0xDE, 0xA4, 0x33, 0xC0, 0x3C),
				Buffer.of(0x53, 0x33, 0x98, 0x67, 0x55, 0x40, 0x33, 0x22),
				Buffer.of(0x05, 0x23, 0xF4, 0x5C, 0xD2, 0xE3, 0xE2, 0xEE),
				Buffer.of(0x09, 0xFF, 0xFF, 0x0F, 0xF0, 0x10, 0xA0, 0x02)
			]);

			return {
				buffer: Buffer.concat([
					transactionsHash, // transactionsHash 32 bytes
					Buffer.of(0x00, 0x00, 0x00, 0x00), // payload size 4b
					Buffer.of(0x00, 0x00, 0x00, 0x00) // aggregate transaction header reserved 1 4b
				]),

				// notice that payloadSize, like size, should not be in returned object
				object: {
					transactionsHash,
					aggregateTransactionHeader_Reserved1: 0
				}
			};
		};

		const innerAggregateTxPaddingSize = innerTransactionSize => {
			const alignment = 8;
			return 0 === innerTransactionSize % alignment ? 0 : alignment - (innerTransactionSize % alignment);
		};

		const generateTransaction = options => {
			const type = (options || {}).type || constants.knownTxType;
			const extraSize = (options || {}).extraSize || 0;

			const SignerPublicKey_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signerPublicKey));
			return {
				buffer: Buffer.concat([
					test.buffer.fromSize(constants.sizes.embedded + extraSize),
					Buffer.of(0x00, 0x00, 0x00, 0x00), // embedded transaction header reserved 1 4b
					SignerPublicKey_Buffer,
					Buffer.of(0x00, 0x00, 0x00, 0x00), // entity body reserved 1
					Buffer.of(0x2A), // version 1b
					Buffer.of(0x55), // network 1b
					Buffer.of(type & 0xFF, (type >> 8) & 0xFF), // type 2b
					Buffer.of(0x46, 0x8B, 0x15, 0x2D), // alpha
					Buffer.of(extraSize, 0x30, 0xE8, 0x50), // beta
					Buffer.alloc(extraSize)
				]),
				object: {
					embeddedTransactionHeader_Reserved1: 0,
					signerPublicKey: SignerPublicKey_Buffer,
					entityBody_Reserved1: 0,
					version: 0x2A,
					network: 0x55,
					type,
					alpha: 0x2D158B46,
					beta: 0x50E83000 | extraSize
				}
			};
		};

		const addTransaction = (generator, options) => () => {
			const data = generator();
			const txData = generateTransaction(options);
			const txPadding = innerAggregateTxPaddingSize(txData.buffer.length);
			data.buffer = Buffer.concat([
				data.buffer,
				txData.buffer
			]);

			if (txPadding)
				data.buffer = Buffer.concat([data.buffer, Buffer.alloc(txPadding)]);

			const payloadSize = data.buffer.readUInt32LE(constants.sizes.transactionsHash) + txData.buffer.length + txPadding;
			data.buffer.writeUInt32LE(payloadSize, constants.sizes.transactionsHash);

			if (!data.object.transactions)
				data.object.transactions = [];

			data.object.transactions.push({ transaction: txData.object });
			return data;
		};

		const addCosignature = generator => {
			const Version_Buffer = Buffer.of(0x46, 0x8B, 0x15, 0x2D, 0x30, 0xE8, 0x50, 0x54);
			const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signerPublicKey));
			const Signature_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signature));

			return () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Version_Buffer,
					Signer_Buffer,
					Signature_Buffer
				]);

				if (!data.object.cosignatures)
					data.object.cosignatures = [];

				data.object.cosignatures.push({
					version: uint64.fromBytes(Version_Buffer),
					signerPublicKey: Signer_Buffer,
					signature: Signature_Buffer
				});
				return data;
			};
		};

		const addAggregateTests = (aggregateName, getCodec) => {
			describe(`supports ${aggregateName} aggregate`, () => {
				const addAll = (size, dataGenerator) => {
					// notice that the transaction header is preprocessed before deserialize is called on the codec
					test.binary.test.addAll(getCodec(), size, dataGenerator, getSubTxCodecs(), constants.sizes.transaction);
				};

				describe('with neither transactions nor cosignatures', () => {
					addAll(constants.sizes.aggregate, generateAggregate);
				});

				describe('with single transaction', () => {
					addAll(
						constants.sizes.aggregate + constants.sizes.embedded,
						addTransaction(generateAggregate)
					);
				});

				describe('with multiple transactions', () => {
					// use extraSize to emulate transactions of varying sizes within a single aggregate
					const extraSize1 = 1;
					const extraSize2 = 4;
					const extraSize3 = 2;

					addAll(
						constants.sizes.aggregate + (3 * constants.sizes.embedded) + (extraSize1 + extraSize2 + extraSize3)
							+ (innerAggregateTxPaddingSize(constants.sizes.aggregate + extraSize1)
								+ innerAggregateTxPaddingSize(constants.sizes.aggregate + extraSize2)
								+ innerAggregateTxPaddingSize(constants.sizes.aggregate + extraSize3)),
						addTransaction(
							addTransaction(
								addTransaction(generateAggregate, { extraSize: extraSize1 }),
								{ extraSize: extraSize2 }
							),
							{ extraSize: extraSize3 }
						)
					);
				});

				describe('with cosignatures', () => {
					addAll(
						constants.sizes.aggregate + (2 * constants.sizes.cosignature),
						addCosignature(addCosignature(generateAggregate))
					);
				});

				describe('with multiple transactions and cosignatures', () => {
					addAll(
						constants.sizes.aggregate + (3 * constants.sizes.embedded) + (2 * constants.sizes.cosignature),
						addCosignature(addCosignature(addTransaction(addTransaction(addTransaction(generateAggregate)))))
					);
				});
			});

			describe(`rejects ${aggregateName} aggregate`, () => {
				describe('during deserialization if it', () => {
					it('is embedded', () => {
						// Arrange:
						const codec = getCodec();
						const parser = new BinaryParser();
						parser.push(generateAggregate().buffer);

						// Act: calling deserialize without tx codecs emulates an embedded call
						expect(() => { codec.deserialize(parser); }).to.throw('aggregate transaction is not embeddable');
					});

					const assertDeserializationError = (buffer, size, errorText, message) => {
						// Arrange:
						const codec = getCodec();
						const parser = new BinaryParser();
						parser.push(buffer);

						// Act:
						expect(() => { codec.deserialize(parser, size, getSubTxCodecs()); }, message).to.throw(errorText);
					};

					it('has sub transaction of unknown type', () => {
						// Assert:
						assertDeserializationError(
							addTransaction(generateAggregate, { type: 0x0001 })().buffer,
							constants.sizes.aggregate + constants.sizes.embedded,
							'error unsupported transaction type (1) in aggregate'
						);
					});

					it('has partial cosignatures', () => {
						// Arrange:
						[-1, 1].forEach(delta => {
							// Assert:
							assertDeserializationError(
								addCosignature(addTransaction(generateAggregate))().buffer,
								constants.sizes.aggregate + constants.sizes.embedded + constants.sizes.cosignature + delta,
								'aggregate cannot have partial cosignatures',
								`delta ${delta}`
							);
						});
					});

					it('fails if payload size is too large', () => {
						// Arrange:
						[1, constants.sizes.aggregate, constants.sizes.aggregate + 1, constants.sizes.embedded].forEach(delta => {
							// - increase reported payload size
							const data = addTransaction(addTransaction(addTransaction(generateAggregate)))();
							data.buffer.writeUInt32LE(
								data.buffer.readUInt32LE(constants.sizes.transactionsHash) + delta,
								constants.sizes.transactionsHash
							);

							// Assert:
							assertDeserializationError(
								data.buffer,
								constants.sizes.aggregate + (3 * constants.sizes.embedded),
								'aggregate must contain complete payload',
								`delta ${delta}`
							);
						});
					});

					it('fails if aggregate size is too small', () => {
						// Arrange:
						[0, 1, constants.sizes.aggregate - 1].forEach(size => {
							// Assert:
							assertDeserializationError(
								addCosignature(addTransaction(generateAggregate))().buffer,
								size,
								'aggregate must contain complete aggregate header',
								`size ${size}`
							);
						});
					});

					it('fails if sub transaction size is too small', () => {
						// Arrange:
						[0, 1, constants.sizes.embedded - 8 - 1].forEach(size => {
							// - modify second transaction size (notice that data.buffer does not include aggregate transaction header)
							const data = addTransaction(addTransaction(addTransaction(generateAggregate)))();
							const offset = constants.sizes.aggregate - constants.sizes.transaction + constants.sizes.embedded;
							data.buffer.writeUInt32LE(size, offset);

							// Assert:
							assertDeserializationError(
								data.buffer,
								constants.sizes.aggregate + (3 * constants.sizes.embedded),
								'sub transaction must contain complete transaction header',
								`size ${size}`
							);
						});
					});
				});

				describe('during serialization if it', () => {
					it('is embedded', () => {
						// Arrange:
						const codec = getCodec();
						const { object } = generateAggregate();
						const serializer = new BinarySerializer(constants.sizes.aggregate);

						// Act: calling serialize without tx codecs emulates an embedded call
						expect(() => { codec.serialize(object, serializer); }).to.throw('aggregate transaction is not embeddable');
					});

					it('has sub transaction of unknown type', () => {
						// Arrange:
						const codec = getCodec();
						const { object } = addTransaction(generateAggregate, { type: 0x0001 })();
						const serializer = new BinarySerializer(constants.sizes.aggregate);

						// Act:
						expect(() => { codec.serialize(object, serializer, getSubTxCodecs()); })
							.to.throw('error unsupported transaction type (1) in aggregate');
					});
				});
			});
		};

		addAggregateTests('complete', () => getCodecs()[EntityType.aggregateComplete]);
		addAggregateTests('bonded', () => getCodecs()[EntityType.aggregateBonded]);
	});
});
