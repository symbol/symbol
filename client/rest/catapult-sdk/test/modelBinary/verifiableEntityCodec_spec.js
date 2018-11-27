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

const test = require('../binaryTestUtils');
const verifiableEntityCodec = require('../../src/modelBinary/verifiableEntityCodec');

describe('verifiable entity codec', () => {
	const generateVerifiableEntity = () => {
		const Signature_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signature));
		const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signer));

		return {
			buffer: Buffer.concat([
				Signature_Buffer,
				Signer_Buffer,
				Buffer.of(0x2A, 0x81, 0x1C, 0x45) // version, type
			]),
			object: {
				signature: Signature_Buffer,
				signer: Signer_Buffer,
				version: 0x812A,
				type: 0x451C
			}
		};
	};

	test.binary.test.addAll(verifiableEntityCodec, 100, generateVerifiableEntity);
});
