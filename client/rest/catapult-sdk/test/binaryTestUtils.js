import { expect } from 'chai';
import BinaryParser from '../src/parser/BinaryParser';
import BinarySerializer from '../src/serializer/BinarySerializer';
import test from './testUtils';

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

		assertRoundtrip(codec, size, object, txCodecs) {
			// Arrange: serialize an object
			const serializer = new BinarySerializer(size);
			codec.serialize(object, serializer, txCodecs);

			// Act: deserialize it
			const parser = new BinaryParser();
			parser.push(serializer.buffer());
			const model = codec.deserialize(parser, size, txCodecs);

			// Assert:
			expect(model).to.deep.equal(object);
		},

		test: {
			addAll(codec, size, dataGenerator, txCodecs) {
				it('can be serialized', () => {
					// Arrange:
					const data = dataGenerator();

					// Assert:
					binaryTestUtils.binary.assertSerialization(codec, size, data.object, data.buffer, txCodecs);
				});

				it('can be deserialized', () => {
					// Arrange:
					const data = dataGenerator();

					// Assert:
					binaryTestUtils.binary.assertDeserialization(codec, size, data.buffer, data.object, txCodecs);
				});

				it('can be roundtripped', () => {
					// Arrange:
					const data = dataGenerator();

					// Assert:
					binaryTestUtils.binary.assertRoundtrip(codec, size, data.object, txCodecs);
				});
			}
		}
	}
};

Object.assign(binaryTestUtils, test);

export default binaryTestUtils;
