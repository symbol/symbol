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

const blockHeaderCodec = require('../../src/modelBinary/blockHeaderCodec');
const test = require('../binaryTestUtils');

describe('block header codec', () => {
	const generateBlockHeader = () => {
		const Previous_Block_Hash_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));
		const Block_Transactions_Hash_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash256));

		return {
			buffer: Buffer.concat([
				Buffer.of(0x97, 0x87, 0x45, 0x0E, 0xE1, 0x6C, 0xB6, 0x62), // height
				Buffer.of(0x30, 0x3A, 0x46, 0x8B, 0x15, 0x2D, 0x60, 0x54), // timestamp
				Buffer.of(0x86, 0x02, 0x75, 0x30, 0xE8, 0x50, 0x78, 0xE8), // difficulty
				Previous_Block_Hash_Buffer,
				Block_Transactions_Hash_Buffer
			]),
			object: {
				height: [0x0E458797, 0x62B66CE1],
				timestamp: [0x8B463A30, 0x54602D15],
				difficulty: [0x30750286, 0xE87850E8],
				previousBlockHash: Previous_Block_Hash_Buffer,
				blockTransactionsHash: Block_Transactions_Hash_Buffer
			}
		};
	};

	test.binary.test.addAll(blockHeaderCodec, 88, generateBlockHeader);
});
