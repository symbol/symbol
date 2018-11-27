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

const convert = require('../../src/utils/convert');
const serialize = require('../../src/modelBinary/serialize');
const { expect } = require('chai');

describe('serialize', () => {
	const getCodec = () => ({
		deserialize: parser => {
			const transaction = {};
			transaction.alpha = parser.uint16();
			transaction.beta = parser.uint32();
			return transaction;
		},

		serialize: (transaction, serializer) => {
			serializer.writeUint16(transaction.alpha);
			serializer.writeUint32(transaction.beta);
		}
	});

	const runTest = actAndAssert => {
		// Arrange:
		const expectedBuffer = Buffer.concat([
			Buffer.of(0x46, 0x8B), // alpha
			Buffer.of(0xFE, 0x30, 0xE8, 0x50) // beta
		]);

		const transaction = {
			alpha: 0x8B46,
			beta: 0x50E830FE
		};

		// Act + Assert:
		actAndAssert(expectedBuffer, transaction);
	};

	it('to buffer returns appropriate buffer', () => {
		// Arrange:
		runTest((expectedBuffer, transaction) => {
			// Act:
			const buffer = serialize.toBuffer(getCodec(), transaction);

			// Assert:
			expect(buffer).to.deep.equal(expectedBuffer);
		});
	});

	it('to hex returns appropriate hex string', () => {
		// Arrange:
		runTest((expectedBuffer, transaction) => {
			const expectedHex = convert.uint8ToHex(expectedBuffer);

			// Act:
			const hex = serialize.toHex(getCodec(), transaction);

			// Assert:
			expect(hex).to.equal(expectedHex);
		});
	});
});
