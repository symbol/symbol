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

const embeddedEntityCodec = require('../../src/modelBinary/embeddedEntityCodec');
const test = require('../binaryTestUtils');

describe('embedded entity codec', () => {
	const generateEmbeddedEntity = () => {
		const SignerPublicKey_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signerPublicKey));

		return {
			buffer: Buffer.concat([
				Buffer.of(0x00, 0x00, 0x00, 0x00), // embedded transaction header reserved 1 4b
				SignerPublicKey_Buffer,
				Buffer.of(0x00, 0x00, 0x00, 0x00), // entity body reserved 1 4b
				Buffer.of(0x2A), // version 1b
				Buffer.of(0x55), // network 1b
				Buffer.of(0x1C, 0x45) // type 2b
			]),
			object: {
				embeddedTransactionHeader_Reserved1: 0,
				signerPublicKey: SignerPublicKey_Buffer,
				entityBody_Reserved1: 0,
				version: 0x2A,
				network: 0x55,
				type: 0x451C
			}
		};
	};

	test.binary.test.addAll(embeddedEntityCodec, 44, generateEmbeddedEntity);
});
