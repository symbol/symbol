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

const packetHeader = require('../../src/packet/header');
const { expect } = require('chai');

describe('packet header', () => {
	describe('constants', () => {
		it('has correct size', () => {
			// Act:
			const { size } = packetHeader;

			// Assert:
			expect(size).to.equal(8);
		});
	});

	describe('create buffer', () => {
		it('can create header buffer', () => {
			// Act:
			const buffer = packetHeader.createBuffer(0x1234, 0x987);

			// Assert:
			expect(buffer).to.deep.equal(Buffer.of(0x87, 0x09, 0x00, 0x00, 0x34, 0x12, 0x00, 0x00));
		});
	});
});
