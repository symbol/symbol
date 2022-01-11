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

const test = require('./testUtils');
const BinaryParser = require('../src/parser/BinaryParser');
const BinarySerializer = require('../src/serializer/BinarySerializer');
const { expect } = require('chai');

const binaryTestUtils = {
	binary: {
		assertSerialization(codec, size, object, buffer, txCodecs) {
			// Arrange: create a serializer
			const serializer = new BinarySerializer(size);

			// Act:
			codec.serialize(object, serializer, txCodecs);

			// Assert:
			expect(serializer.buffer()).to.deep.equal(buffer);
		},

		assertDeserialization(codec, size, buffer, object, txCodecs) {
			// Arrange: create a parser around the buffer
			const parser = new BinaryParser();
			parser.push(buffer);

			// Act:
			const model = codec.deserialize(parser, size, txCodecs);

			// Assert:
			expect(model).to.deep.equal(object);
		},

		assertRoundtrip(codec, size, object, txCodecs, preprocessedHeaderSize) {
			// Arrange: serialize an object
			const serializer = new BinarySerializer(size);
			codec.serialize(object, serializer, txCodecs);

			// Act: deserialize it (the size passed to deserialize should include the preprocessed header size)
			const parser = new BinaryParser();
			parser.push(serializer.buffer());
			const model = codec.deserialize(parser, size + preprocessedHeaderSize, txCodecs);

			// Assert:
			expect(model).to.deep.equal(object);
		},

		test: {
			addAll(codec, size, dataGenerator, txCodecs, preprocessedHeaderSize = 0) {
				const { binary } = binaryTestUtils;

				it('can be serialized', () => {
					// Arrange:
					const data = dataGenerator();

					// Assert:
					binary.assertSerialization(codec, size - preprocessedHeaderSize, data.object, data.buffer, txCodecs);
				});

				it('can be deserialized', () => {
					// Arrange:
					const data = dataGenerator();

					// Assert:
					binary.assertDeserialization(codec, size, data.buffer, data.object, txCodecs);
				});

				it('can be roundtripped', () => {
					// Arrange:
					const data = dataGenerator();

					// Assert:
					binary.assertRoundtrip(codec, size - preprocessedHeaderSize, data.object, txCodecs, preprocessedHeaderSize);
				});
			}
		}
	}
};

Object.assign(binaryTestUtils, test);

module.exports = binaryTestUtils;
