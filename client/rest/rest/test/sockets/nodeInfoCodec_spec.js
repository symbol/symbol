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

const nodeInfoCodec = require('../../src/sockets/nodeInfoCodec');
const catapult = require('catapult-sdk');
const { expect } = require('chai');

const { BinaryParser } = catapult.parser;

describe('deserialize', () => {
	it('returns a deserialized object without friendlyName or host', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const publicKeyBuffer = Buffer.from([
			0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
			0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
		]);
		const networkGenerationHashBuffer = Buffer.from([
			0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
			0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8
		]);
		const packetBuffer = Buffer.concat([
			Buffer.from([0x31, 0x00, 0x00, 0x00]), // size
			Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
			publicKeyBuffer,
			networkGenerationHashBuffer,
			Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
			Buffer.from([0xDC, 0x1E]), // port
			Buffer.from([0x90]), // network identifier
			Buffer.from([0x00]), // host size
			Buffer.from([0x00]) // friendly name size
		]);
		binaryParser.push(packetBuffer);

		// Act:
		const deserializedData = nodeInfoCodec.deserialize(binaryParser);

		// Assert:
		expect(deserializedData).to.deep.equal({
			version: 23,
			publicKey: publicKeyBuffer,
			networkGenerationHashSeed: networkGenerationHashBuffer,
			roles: 2,
			port: 7900,
			networkIdentifier: 144,
			friendlyName: Buffer.from([]),
			host: Buffer.from([])
		});
	});

	it('returns a deserialized object with friendlyName', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const friendlyNameBuffer = Buffer.from([0x10]);
		const publicKeyBuffer = Buffer.from([
			0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
			0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
		]);
		const networkGenerationHashBuffer = Buffer.from([
			0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
			0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8
		]);
		const packetBuffer = Buffer.concat([
			Buffer.from([0x31 + friendlyNameBuffer.length, 0x00, 0x00, 0x00]), // size
			Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
			publicKeyBuffer,
			networkGenerationHashBuffer,
			Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
			Buffer.from([0xDC, 0x1E]), // port
			Buffer.from([0x90]), // network identifier
			Buffer.from([0x00]), // host size
			Buffer.from([friendlyNameBuffer.length]), // friendly name size
			// host
			friendlyNameBuffer
		]);
		binaryParser.push(Buffer.from(packetBuffer));

		// Act:
		const deserializedData = nodeInfoCodec.deserialize(binaryParser);

		// Assert:
		expect(deserializedData).to.deep.equal({
			version: 23,
			publicKey: publicKeyBuffer,
			networkGenerationHashSeed: networkGenerationHashBuffer,
			roles: 2,
			port: 7900,
			networkIdentifier: 144,
			friendlyName: friendlyNameBuffer,
			host: Buffer.from([])
		});
	});

	it('returns a deserialized object with host', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const hostBuffer = Buffer.from([0xCC]);
		const publicKeyBuffer = Buffer.from([
			0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
			0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
		]);
		const networkGenerationHashBuffer = Buffer.from([
			0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
			0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8
		]);
		const packetBuffer = Buffer.concat([
			Buffer.from([0x31 + hostBuffer.length, 0x00, 0x00, 0x00]), // size
			Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
			publicKeyBuffer,
			networkGenerationHashBuffer,
			Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
			Buffer.from([0xDC, 0x1E]), // port
			Buffer.from([0x90]), // network identifier
			Buffer.from([hostBuffer.length]), // host size
			Buffer.from([0x00]), // friendly name size
			hostBuffer
		]);
		binaryParser.push(Buffer.from(packetBuffer));

		// Act:
		const deserializedData = nodeInfoCodec.deserialize(binaryParser);

		// Assert:
		expect(deserializedData).to.deep.equal({
			version: 23,
			publicKey: publicKeyBuffer,
			networkGenerationHashSeed: networkGenerationHashBuffer,
			roles: 2,
			port: 7900,
			networkIdentifier: 144,
			friendlyName: Buffer.from([]),
			host: hostBuffer
		});
	});

	it('returns a deserialized object with friendlyName and host', () => {
		// Arrange:
		const binaryParser = new BinaryParser();
		const friendlyNameBuffer = Buffer.from([0x10, 0x17]);
		const hostBuffer = Buffer.from([0xCC, 0x00, 0x03]);
		const publicKeyBuffer = Buffer.from([
			0xE3, 0x27, 0xC0, 0xF1, 0xC9, 0x97, 0x5C, 0x3A, 0xA5, 0x1B, 0x2A, 0x41, 0x76, 0x81, 0x58, 0xC1,
			0x07, 0x7D, 0x16, 0xB4, 0x60, 0x99, 0x9A, 0xAB, 0xE7, 0xAD, 0xB5, 0x26, 0x2B, 0xE2, 0x9A, 0x68
		]);
		const networkGenerationHashBuffer = Buffer.from([
			0xA3, 0x00, 0xEA, 0xFE, 0xDA, 0xBD, 0x5C, 0xFA, 0x0D, 0x4B, 0x94, 0x1D, 0x15, 0xBB, 0x51, 0xB1,
			0xB4, 0x64, 0x72, 0x42, 0xF1, 0xFF, 0x11, 0x00, 0x9F, 0xD0, 0x9A, 0x8F, 0x3D, 0x35, 0x87, 0xF8
		]);
		const packetBuffer = Buffer.concat([
			Buffer.from([0x31 + friendlyNameBuffer.length + hostBuffer.length, 0x00, 0x00, 0x00]), // size
			Buffer.from([0x17, 0x00, 0x00, 0x00]), // version
			publicKeyBuffer,
			networkGenerationHashBuffer,
			Buffer.from([0x02, 0x00, 0x00, 0x00]), // roles
			Buffer.from([0xDC, 0x1E]), // port
			Buffer.from([0x90]), // network identifier
			Buffer.from([hostBuffer.length]), // host size
			Buffer.from([friendlyNameBuffer.length]), // friendly name size
			hostBuffer,
			friendlyNameBuffer
		]);
		binaryParser.push(Buffer.from(packetBuffer));

		// Act:
		const deserializedData = nodeInfoCodec.deserialize(binaryParser);

		// Assert:
		expect(deserializedData).to.deep.equal({
			version: 23,
			publicKey: publicKeyBuffer,
			networkGenerationHashSeed: networkGenerationHashBuffer,
			roles: 2,
			port: 7900,
			networkIdentifier: 144,
			friendlyName: friendlyNameBuffer,
			host: hostBuffer
		});
	});
});
