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

const transactionCodec = require('../../src/modelBinary/transactionCodec');
const test = require('../binaryTestUtils');

describe('transaction codec', () => {
	const generateTransaction = () => ({
		buffer: Buffer.concat([
			Buffer.of(0x18, 0xA2, 0x46, 0xD0, 0x56, 0xDC, 0x18, 0xB0), // maxFee
			Buffer.of(0x4A, 0xE0, 0xDA, 0x7F, 0x93, 0x73, 0x11, 0xC0) // deadline
		]),
		object: {
			maxFee: [0xD046A218, 0xB018DC56],
			deadline: [0x7FDAE04A, 0xC0117393]
		}
	});

	test.binary.test.addAll(transactionCodec, 16, generateTransaction);
});
