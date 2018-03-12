const { expect } = require('chai');
const BinaryParser = require('../src/parser/BinaryParser');
const BinarySerializer = require('../src/serializer/BinarySerializer');
const test = require('./testUtils');

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
