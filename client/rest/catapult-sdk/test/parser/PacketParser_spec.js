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

const PacketParser = require('../../src/parser/PacketParser');
const { expect } = require('chai');

describe('PacketParser', () => {
	const Test_Buffer_64 = Buffer.of(
		0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD,
		0xEE, 0xEE, 0xFF, 0xFF, 0xAA, 0xAA, 0xBB, 0xBB,
		0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF,
		0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33,
		0xAA, 0xAA, 0xBB, 0xBB, 0xCC, 0xCC, 0xDD, 0xDD,
		0xEE, 0xEE, 0xFF, 0xFF, 0xAA, 0xAA, 0xBB, 0xBB,
		0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF,
		0x12, 0x34, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33
	);

	describe('basic', () => {
		const createPacketParser = packets => {
			const parser = new PacketParser();
			parser.onPacket(packet => packets.push(packet));
			return parser;
		};

		it('fails if packet sizes are invalid', () => {
			// Arrange:
			const packets = [];
			const parser = createPacketParser(packets);

			// Act:
			const errorMessageText = 'cannot be less than packet header size';
			expect(() => { parser.push(Buffer.from('0000000004000000', 'hex')); }, '0').to.throw(errorMessageText);
			expect(() => { parser.push(Buffer.from('0700000004000000', 'hex')); }, '7').to.throw(errorMessageText);

			// Assert:
			expect(packets.length).to.equal(0);
		});

		it('succeeds if packet has min size', () => {
			// Arrange:
			const packets = [];
			const parser = createPacketParser(packets);

			// Act:
			parser.push(Buffer.from('0800000009000000', 'hex'));

			// Assert:
			expect(packets.length).to.equal(1);
			expect(packets[0]).to.deep.equal({ type: 0x0009, size: 0x0008, payload: Buffer.from([]) });
		});

		it('succeeds if packet has nonzero payload size', () => {
			// Arrange:
			const packets = [];
			const parser = createPacketParser(packets);

			// Act:
			parser.push(Buffer.from('1600000004000000AAAABBBBCCCCDDDDEEEEFFFFAAAA', 'hex'));

			// Assert:
			expect(packets.length).to.equal(1);
			expect(packets[0]).to.deep.equal({
				type: 0x0004,
				size: 0x0016,
				payload: Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAA', 'hex')
			});
		});

		it('can parse packet spanning buffers', () => {
			// Arrange:
			const packets = [];
			const parser = createPacketParser(packets);

			// Act:
			parser.push(Buffer.from([0x48, 0x00, 0x00, 0x00]));
			parser.push(Buffer.from([0x01, 0x00, 0x00, 0x00]));
			parser.push(Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFF0000111122223333', 'hex'));
			parser.push(Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFF1234111122223333', 'hex'));

			// Assert:
			expect(packets.length).to.equal(1);
			expect(packets[0]).to.deep.equal({ type: 0x0001, size: 0x0048, payload: Test_Buffer_64 });
		});

		it('can parse multiple packets', () => {
			// Arrange:
			const packets = [];
			const parser = createPacketParser(packets);

			// Act:
			parser.push(Buffer.from('1600000004000000AAAABBBBCCCCDDDDEEEEFFFFAAAA4800000001000000', 'hex'));
			parser.push(Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFF0000111122223333', 'hex'));
			parser.push(Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFF1234111122223333', 'hex'));
			parser.push(Buffer.from('1000000008000000FEDCBA9876543210', 'hex'));

			// Assert:
			expect(packets.length).to.equal(3);
			expect(packets[0]).to.deep.equal({ type: 0x0004, size: 0x0016, payload: Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAA', 'hex') });
			expect(packets[1]).to.deep.equal({ type: 0x0001, size: 0x0048, payload: Test_Buffer_64 });
			expect(packets[2]).to.deep.equal({ type: 0x0008, size: 0x0010, payload: Buffer.from('FEDCBA9876543210', 'hex') });
		});

		it('can parse multiple packets in single buffer', () => {
			// Arrange:
			const packets = [];
			const parser = createPacketParser(packets);

			// Act:
			const hex = '1600000004000000AAAABBBBCCCCDDDDEEEEFFFFAAAA4800000001000000'
				+ 'AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFF0000111122223333'
				+ 'AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDDEEEEFFFF1234111122223333'
				+ '1000000008000000FEDCBA9876543210';
			parser.push(Buffer.from(hex, 'hex'));

			// Assert:
			expect(packets.length).to.equal(3);
			expect(packets[0]).to.deep.equal({ type: 0x0004, size: 0x0016, payload: Buffer.from('AAAABBBBCCCCDDDDEEEEFFFFAAAA', 'hex') });
			expect(packets[1]).to.deep.equal({ type: 0x0001, size: 0x0048, payload: Test_Buffer_64 });
			expect(packets[2]).to.deep.equal({ type: 0x0008, size: 0x0010, payload: Buffer.from('FEDCBA9876543210', 'hex') });
		});
	});
});
