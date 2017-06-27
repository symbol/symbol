import { expect } from 'chai';
import aggregate from '../../src/plugins/aggregate';
import EntityType from '../../src/model/EntityType';
import ModelSchemaBuilder from '../../src/model/ModelSchemaBuilder';
import ModelType from '../../src/model/ModelType';
import BinaryParser from '../../src/parser/BinaryParser';
import BinarySerializer from '../../src/serializer/BinarySerializer';
import test from '../binaryTestUtils';

const constants = {
	knownTxType: 0x0022,
	sizes: {
		aggregate: 4,
		embedded: 40 + 8,
		cosignature: 96
	}
};

describe('aggregate plugin', () => {
	describe('register schema', () => {
		it('adds aggregate system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			aggregate.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 2);
			expect(modelSchema).to.contain.all.keys(['aggregate', 'aggregate.cosignature']);

			// - aggregate
			expect(Object.keys(modelSchema.aggregate).length).to.equal(Object.keys(modelSchema.transaction).length + 2);
			expect(modelSchema.aggregate).to.contain.all.keys(['transactions', 'cosignatures']);

			// - cosignature
			expect(modelSchema['aggregate.cosignature']).to.deep.equal({
				signer: ModelType.binary,
				signature: ModelType.binary
			});
		});
	});

	describe('register codecs', () => {
		function getCodecs() {
			const codecs = {};
			aggregate.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		}

		it('adds aggregate codec', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codec was registered
			expect(Object.keys(codecs).length).to.equal(1);
			expect(codecs).to.contain.all.keys([EntityType.aggregate.toString()]);
		});

		function getSubTxCodecs() {
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
		}

		function generateAggregate() {
			return {
				buffer: Buffer.concat([
					Buffer.of(0x00, 0x00, 0x00, 0x00) // payload size
				]),

				// notice that payloadSize, like size, should not be in returned object
				object: {
				}
			};
		}

		function generateTransaction(options) {
			const type = (options || {}).type || constants.knownTxType;
			const extraSize = (options || {}).extraSize || 0;

			const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signer));
			return {
				buffer: Buffer.concat([
					test.buffer.fromSize(constants.sizes.embedded + extraSize),
					Signer_Buffer,
					Buffer.of(0x2A, 0x81, type & 0xFF, (type >> 8) & 0xFF), // version, type
					Buffer.of(0x46, 0x8B, 0x15, 0x2D), // alpha
					Buffer.of(extraSize, 0x30, 0xE8, 0x50), // beta
					Buffer.alloc(extraSize)
				]),
				object: {
					signer: Signer_Buffer,
					version: 0x812A,
					type,

					alpha: 0x2D158B46,
					beta: 0x50E83000 | extraSize
				}
			};
		}

		function getCodec() {
			return getCodecs()[EntityType.aggregate];
		}

		function addTransaction(generator, options) {
			return () => {
				const data = generator();
				const txData = generateTransaction(options);
				data.buffer = Buffer.concat([
					data.buffer,
					txData.buffer
				]);

				const payloadSize = data.buffer.readUInt32LE(0) + constants.sizes.embedded + ((options || {}).extraSize || 0);
				data.buffer.writeUInt32LE(payloadSize, 0);

				if (!data.object.transactions)
					data.object.transactions = [];

				data.object.transactions.push(txData.object);
				return data;
			};
		}

		function addCosignature(generator) {
			const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signer));
			const Signature_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signature));

			return () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Signer_Buffer,
					Signature_Buffer
				]);

				if (!data.object.cosignatures)
					data.object.cosignatures = [];

				data.object.cosignatures.push({
					signer: Signer_Buffer,
					signature: Signature_Buffer
				});
				return data;
			};
		}

		describe('supports aggregate', () => {
			describe('with neither transactions nor cosignatures', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.aggregate, generateAggregate, getSubTxCodecs());
			});

			describe('with single transaction', () => {
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.aggregate + constants.sizes.embedded,
					addTransaction(generateAggregate),
					getSubTxCodecs());
			});

			describe('with multiple transactions', () => {
				// use extraSize to emulate transactions of varying sizes within a single aggregate
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.aggregate + (3 * constants.sizes.embedded) + 7,
					addTransaction(addTransaction(addTransaction(generateAggregate, { extraSize: 1 }), { extraSize: 4 }), { extraSize: 2 }),
					getSubTxCodecs());
			});

			describe('with cosignatures', () => {
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.aggregate + (2 * constants.sizes.cosignature),
					addCosignature(addCosignature(generateAggregate)),
					getSubTxCodecs());
			});

			describe('with multiple transactions and cosignatures', () => {
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.aggregate + (3 * constants.sizes.embedded) + (2 * constants.sizes.cosignature),
					addCosignature(addCosignature(addTransaction(addTransaction(addTransaction(generateAggregate))))),
					getSubTxCodecs());
			});
		});

		describe('rejects aggregate', () => {
			describe('during deserialization if it', () => {
				it('is embedded', () => {
					// Arrange:
					const codec = getCodec();
					const parser = new BinaryParser();
					parser.push(generateAggregate().buffer);

					// Act: calling deserialize without tx codecs emulates an embedded call
					expect(() => { codec.deserialize(parser); }).to.throw('aggregate transaction is not embeddable');
				});

				function assertDeserializationError(buffer, size, errorText, message) {
					// Arrange:
					const codec = getCodec();
					const parser = new BinaryParser();
					parser.push(buffer);

					// Act:
					expect(() => { codec.deserialize(parser, size, getSubTxCodecs()); }, message).to.throw(errorText);
				}

				it('has sub transaction of unknown type', () => {
					// Assert:
					assertDeserializationError(
						addTransaction(generateAggregate, { type: 0x0001 })().buffer,
						constants.sizes.aggregate + constants.sizes.embedded,
						'error unsupported transaction type (1) in aggregate');
				});

				it('has partial cosignatures', () => {
					// Arrange:
					for (const delta of [-1, 1]) {
						// Assert:
						assertDeserializationError(
							addCosignature(addTransaction(generateAggregate))().buffer,
							constants.sizes.aggregate + constants.sizes.embedded + constants.sizes.cosignature + delta,
							'aggregate cannot have partial cosignatures',
							`delta ${delta}`);
					}
				});

				it('fails if payload size is too large', () => {
					// Arrange:
					for (const delta of [1, constants.sizes.aggregate, constants.sizes.aggregate + 1, constants.sizes.embedded]) {
						// - increase reported payload size
						const data = addTransaction(addTransaction(addTransaction(generateAggregate)))();
						data.buffer.writeUInt32LE(data.buffer.readUInt32LE() + delta);

						// Assert:
						assertDeserializationError(
							data.buffer,
							constants.sizes.aggregate + (3 * constants.sizes.embedded),
							'aggregate must contain complete payload',
							`delta ${delta}`);
					}
				});

				it('fails if aggregate size is too small', () => {
					// Arrange:
					for (const size of [0, 1, constants.sizes.aggregate - 1]) {
						// Assert:
						assertDeserializationError(
							addCosignature(addTransaction(generateAggregate))().buffer,
							size,
							'aggregate must contain complete aggregate header',
							`size ${size}`);
					}
				});

				it('fails if sub transaction size is too small', () => {
					// Arrange:
					for (const size of [0, 1, constants.sizes.embedded - 8 - 1]) {
						// - modify second transaction size
						const data = addTransaction(addTransaction(addTransaction(generateAggregate)))();
						data.buffer.writeUInt32LE(size, constants.sizes.aggregate + constants.sizes.embedded);

						// Assert:
						assertDeserializationError(
							data.buffer,
							constants.sizes.aggregate + (3 * constants.sizes.embedded),
							'sub transaction must contain complete transaction header',
							`size ${size}`);
					}
				});
			});

			describe('during serialization if it', () => {
				it('is embedded', () => {
					// Arrange:
					const codec = getCodec();
					const object = generateAggregate().object;
					const serializer = new BinarySerializer(constants.sizes.aggregate);

					// Act: calling serialize without tx codecs emulates an embedded call
					expect(() => { codec.serialize(object, serializer); }).to.throw('aggregate transaction is not embeddable');
				});

				it('has sub transaction of unknown type', () => {
					// Arrange:
					const codec = getCodec();
					const object = addTransaction(generateAggregate, { type: 0x0001 })().object;
					const serializer = new BinarySerializer(constants.sizes.aggregate);

					// Act:
					expect(() => { codec.serialize(object, serializer, getSubTxCodecs()); })
						.to.throw('error unsupported transaction type (1) in aggregate');
				});
			});
		});
	});
});
