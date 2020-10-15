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

const blockHeaderCodec = require('../../src/modelBinary/blockHeaderCodec');
const test = require('../binaryTestUtils');

describe('block header codec', () => {
	const generateBlockHeader = () => {
		const proofGamma = Buffer.from(test.random.bytes(test.constants.sizes.vrfProof.gamma)); // 32b
		const proofVerificationHash = Buffer.from(test.random.bytes(test.constants.sizes.vrfProof.verificationHash)); // 16b
		const proofScalar = Buffer.from(test.random.bytes(test.constants.sizes.vrfProof.scalar)); // 32b
		const previousBlockHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const transactionsHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const receiptsHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const stateHashBuffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const beneficiaryAddress = test.random.bytes(test.constants.sizes.addressDecoded); // 24
		const feeMultiplierBuffer = Buffer.of(0x0A, 0x00, 0x00, 0x00);

		return {
			buffer: Buffer.concat([
				Buffer.of(0x97, 0x87, 0x45, 0x0E, 0xE1, 0x6C, 0xB6, 0x62), // height 8b
				Buffer.of(0x30, 0x3A, 0x46, 0x8B, 0x15, 0x2D, 0x60, 0x54), // timestamp 8b
				Buffer.of(0x86, 0x02, 0x75, 0x30, 0xE8, 0x50, 0x78, 0xE8), // difficulty 8b
				proofGamma, // 32b
				proofVerificationHash, // 16b
				proofScalar, // 32b
				previousBlockHashBuffer, // 32b
				transactionsHashBuffer, // 32b
				receiptsHashBuffer, // 32b
				stateHashBuffer, // 32b
				Buffer.from(beneficiaryAddress), // address 24b
				feeMultiplierBuffer, // 4b
				Buffer.of(0x00, 0x00, 0x00, 0x00) // block header reserved 1 4b
			]),
			object: {
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
			}
		};
	};

	test.binary.test.addAll(blockHeaderCodec, 264, generateBlockHeader);
});
