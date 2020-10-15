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

const ModelCodecBuilder = require('../../src/modelBinary/ModelCodecBuilder');
const BinaryParser = require('../../src/parser/BinaryParser');
const test = require('../binaryTestUtils');
const { expect } = require('chai');

const constants = {
	knownTxType: 0x4123,
	sizes: {
		blockHeader: 376,
		transactionHeader: 128,
		transaction: 128 + 8 + 1
	}
};

describe('model codec builder', () => {
	const countCodecs = codecs => codecs.reduce((count, value) => count + (undefined === value ? 0 : 1), 0);

	const getCodec = () => {
		const builder = new ModelCodecBuilder();
		builder.addTransactionSupport(constants.knownTxType, {
			deserialize: (parser, size, txCodecs) => {
				const transaction = {};
				transaction.alpha = parser.uint32();
				transaction.beta = parser.uint32();

				// store deserialize parameter data in the transaction in order to ensure builder is passing down correct data
				Object.assign(transaction, {
					numSerializeCodecs: parser.uint8(),
					numDeserializeCodecs: countCodecs(txCodecs),
					size
				});
				return transaction;
			},

			serialize: (transaction, serializer, txCodecs) => {
				serializer.writeUint32(transaction.alpha);
				serializer.writeUint32(transaction.beta);

				// write serialize parameter data in the buffer in order to ensure builder is passing down correct data
				// (notice that conditionally writing ensures codecs are passed to size calculator as well
				//  since numCodecs is expected to be greater than 0)
				const numCodecs = countCodecs(txCodecs);
				if (0 < numCodecs)
					serializer.writeUint8(countCodecs(txCodecs));
			}
		});

		return builder.build();
	};

	const generateVerifiableEntity = (size, type = 0x451C) => {
		const Signature_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signature));
		const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signerPublicKey));

		return {
			buffer: Buffer.concat([
				test.buffer.fromSize(size),
				Buffer.of(0x00, 0x00, 0x00, 0x00), // verifiable entity header reserved 1 4b
				Signature_Buffer,
				Signer_Buffer,
				Buffer.of(0x00, 0x00, 0x00, 0x00), // entity body reserved 1 4b
				Buffer.of(0x2A), // version 1b
				Buffer.of(0x81), // network 1b
				Buffer.of(type & 0xFF, (type >> 8) & 0xFF) // type 2b
			]),
			object: {
				verifiableEntityHeader_Reserved1: 0,
				signature: Signature_Buffer,
				signerPublicKey: Signer_Buffer,
				entityBody_Reserved1: 0,
				version: 0x2A,
				network: 0x81,
				type
			}
		};
	};

	const generateBlockHeader = () => {
		const proofGamma = Buffer.from(test.random.bytes(test.constants.sizes.vrfProof.gamma));
		const proofVerificationHash = Buffer.from(test.random.bytes(test.constants.sizes.vrfProof.verificationHash));
		const proofScalar = Buffer.from(test.random.bytes(test.constants.sizes.vrfProof.scalar));
		const previousBlockHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const transactionsHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const receiptsHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const stateHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const beneficiaryAddress = test.random.bytes(test.constants.sizes.addressDecoded); // 24
		const feeMultiplierBuffer = Buffer.of(0x0A, 0x00, 0x00, 0x00);
		const reservedPadding = Buffer.of(0x00, 0x00, 0x00, 0x00);

		const data = generateVerifiableEntity(constants.sizes.blockHeader, 0x8000);
		data.buffer = Buffer.concat([
			data.buffer,
			Buffer.of(0x97, 0x87, 0x45, 0x0E, 0xE1, 0x6C, 0xB6, 0x62), // height
			Buffer.of(0x30, 0x3A, 0x46, 0x8B, 0x15, 0x2D, 0x60, 0x54), // timestamp
			Buffer.of(0x86, 0x02, 0x75, 0x30, 0xE8, 0x50, 0x78, 0xE8), // difficulty
			proofGamma, // 32b
			proofVerificationHash, // 16b
			proofScalar, // 32b
			previousBlockHashBuffer, // 32b
			transactionsHashBuffer, // 32b
			receiptsHashBuffer, // 32b
			stateHashBuffer, // 32b
			Buffer.from(beneficiaryAddress), // address 24b
			feeMultiplierBuffer, // 4b
			reservedPadding // 4b
		]);

		Object.assign(data.object, {
			height: [0x0E458797, 0x62B66CE1],
			timestamp: [0x8B463A30, 0x54602D15],
			difficulty: [0x30750286, 0xE87850E8],
			proofGamma,
			proofVerificationHash,
			proofScalar,
			previousBlockHash: previousBlockHashBuffer,
			transactionsHash: transactionsHashBuffer,
			receiptsHash: receiptsHashBuffer,
			stateHash: stateHashBuffer,
			beneficiaryAddress,
			feeMultiplier: 10,
			blockHeader_Reserved1: 0
		});
		return data;
	};

	const generateTransaction = (type = constants.knownTxType, tag = 0x75) => {
		const data = generateVerifiableEntity(constants.sizes.transaction, type);
		data.buffer = Buffer.concat([
			data.buffer,
			Buffer.of(0x18, 0xA2, 0x46, 0xD0, 0x56, 0xDC, 0x18, 0xB0), // maxFee
			Buffer.of(0x4A, 0xE0, 0xDA, 0x7F, 0x93, 0x73, 0x11, 0xC0), // deadline
			Buffer.of(0x46, 0x8B, 0x15, 0x2D), // alpha
			Buffer.of(tag, 0x30, 0xE8, 0x50), // beta
			Buffer.of(0x01) // placeholder
		]);

		Object.assign(data.object, {
			maxFee: [0xD046A218, 0xB018DC56],
			deadline: [0x7FDAE04A, 0xC0117393],
			alpha: 0x2D158B46,
			beta: 0x50E83000 | tag
		});

		if (constants.knownTxType === type) {
			// add calculated fields in order ensure that the builder passes down correct data to tx codecs
			// (this is not advisable in practice outside of this test suite)
			Object.assign(data.object, {
				numSerializeCodecs: 1,
				numDeserializeCodecs: 1,
				size: constants.sizes.transaction
			});
		}

		return data;
	};

	describe('block', () => {
		test.binary.test.addAll(getCodec(), constants.sizes.blockHeader, generateBlockHeader);
	});

	describe('transaction extension', () => {
		test.binary.test.addAll(getCodec(), constants.sizes.transaction, generateTransaction);
	});

	describe('basic', () => {
		it('block types are supported by default', () => {
			// Act:
			const builder = new ModelCodecBuilder();
			const codec = builder.build();

			// Assert:
			expect(codec.supports(0x8000)).to.equal(true);
			expect(codec.supports(0x8FFF)).to.equal(true);
		});

		it('transaction types are not supported by default', () => {
			// Act:
			const builder = new ModelCodecBuilder();
			const codec = builder.build();

			// Assert:
			expect(codec.supports(0x4000)).to.equal(false);
			expect(codec.supports(0x4123)).to.equal(false);
			expect(codec.supports(0x4FFF)).to.equal(false);
		});

		it('transaction types support can be added', () => {
			// Act:
			const builder = new ModelCodecBuilder();
			builder.addTransactionSupport(0x4123, {});
			const codec = builder.build();

			// Assert:
			expect(codec.supports(0x4000)).to.equal(false);
			expect(codec.supports(0x4123)).to.equal(true);
			expect(codec.supports(0x4FFF)).to.equal(false);
		});

		it('cannot add conflicting extensions', () => {
			// Act:
			const builder = new ModelCodecBuilder();
			builder.addTransactionSupport(0x4000, {});

			// Assert:
			expect(() => builder.addTransactionSupport(0x4000, {})).to.throw('already registered');
		});

		it('cannot override default extensions', () => {
			// Act:
			const builder = new ModelCodecBuilder();

			// Assert:
			[0x8000, 0x8FFF].forEach(type => {
				expect(() => builder.addTransactionSupport(type, {}), type).to.throw('already registered');
			});
		});

		it('cannot serialize unknown model', () => {
			// Arrange:
			const codec = getCodec();

			// Assert:
			expect(() => codec.serialize({ type: 0x4143 }, {})).to.throw('no codec registered');
		});

		it('cannot deserialize unknown model', () => {
			// Arrange:
			const codec = getCodec();
			const parser = new BinaryParser();
			parser.push(generateTransaction(0x4143).buffer);

			// Assert:
			expect(() => codec.deserialize(parser)).to.throw('no codec registered');
		});
	});
});
