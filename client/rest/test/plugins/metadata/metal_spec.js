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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.	If not, see <http://www.gnu.org/licenses/>.
 */

const {
	combinePayloadWithText,
	generateChecksum,
	getMetadata,
	testData
} = require('./metalUtils');
const { hexToUint8 } = require('../../../src/catapult-sdk/utils/convert');
const { metal, MetalSeal } = require('../../../src/plugins/metadata/metal');
const { expect } = require('chai');

describe('metal', () => {
	describe('extractCompositeHashFromMetalId', () => {
		it('can convert MetalID to CompositeHash', () => {
			// Arrange:
			const metalId = 'FeDr2dAx9GnK8j4w6VDwtBLBbvnvxx7rea4eHrpQvG6hhX';
			// Act + Assert:
			expect(metal.extractCompositeHashFromMetalId(metalId))
				.to.deep.equal(hexToUint8('BBFB2A837D3A9CF993195C2CA417645518A2ED2C30E8B760D3E224EC75F601E6'));
		});

		it('cannot convert MetalID to CompositeHash not starting with metal id header', () => {
			// Arrange:
			const metalId = 'AuDr2dAx9GnK8j4w6VDwtBLBbvnvxx7rea4eHrpQvG6hhX';
			// Act + Assert:
			expect(() => metal.extractCompositeHashFromMetalId(metalId)).to.throw(`'${metalId}' is not a valid metal id`);
		});

		it('cannot convert MetalID to CompositeHash when length is not 34', () => {
			// Arrange:
			const metalId = 'FeDr2dAx9GnK8j4w6VDwtBLBbvnvxx7rea4eHrpQvG6hh';
			// Act + Assert:
			expect(() => metal.extractCompositeHashFromMetalId(metalId)).to.throw(`'${metalId}' is not a valid metal id`);
		});
	});

	describe('generateMetadataKey', () => {
		it('can generate metadata key', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const input = metadata.value;
			// Act + Assert:
			expect(metal.generateMetadataKey(new Uint8Array(input))).to.deep.equal(metadata.scopedMetadataKey);
		});

		it('cannot generate metadata key input is empty', () => {
			// Arrange:
			const input = Buffer.alloc(0);
			// Act + Assert:
			expect(() => metal.generateMetadataKey(input)).to.throw('input must not be empty');
		});
	});

	describe('extractChunk', () => {
		it('can extract chunk', () => {
			// Arrange:
			const metadata = getMetadata(70);
			const nextMetadata = getMetadata(80);
			// Act + Assert:
			expect(metal.extractChunk(Buffer.from(metadata.value))).to.deep.equal({
				magic: 0x00,
				scopedMetadataKey: nextMetadata.scopedMetadataKey,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: false
			});
		});

		it('can extract end chunk', () => {
			// Arrange:
			const metadata = getMetadata(120);
			const { combinedPayload } = combinePayloadWithText(testData.imageBytes);
			const checkSum = generateChecksum(combinedPayload);
			// Act + Assert:
			expect(metal.extractChunk(Buffer.from(metadata.value))).to.deep.equal({
				magic: 0x80,
				scopedMetadataKey: checkSum,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: false
			});
		});

		it('can extract chunk with text', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const nextMetadata = getMetadata(20);
			// Act + Assert:
			expect(metal.extractChunk(Buffer.from(metadata.value))).to.deep.equal({
				magic: 0x00,
				scopedMetadataKey: nextMetadata.scopedMetadataKey,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: true
			});
		});

		it('can extract end chunk with text', () => {
			// Arrange:
			const metadata = getMetadata(250);
			const payload = Buffer.from(Buffer.from('test', 'utf-8'));
			const textSection = new MetalSeal(payload.length, 'text/plain', 'text.text', 'test').stringify();
			const { combinedPayload } = combinePayloadWithText(payload, textSection);
			const checkSum = generateChecksum(combinedPayload);
			// Act + Assert:
			expect(metal.extractChunk(Buffer.from(metadata.value))).to.deep.equal({
				magic: 0x80,
				scopedMetadataKey: checkSum,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: true
			});
		});
	});

	describe('splitChunkPayloadAndText', () => {
		it('can extract payload and text when both present', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const extractChunk = metal.extractChunk(Buffer.from(metadata.value));
			const textSection = new MetalSeal(testData.imageBytes.length, 'image/png', 'image.png', 'test').stringify();
			const imageBytesToSeparatorIndex = testData.imageBytes.slice(0, 966);
			// Act:
			const { chunkPayload, chunkText } = metal.splitChunkPayloadAndText(extractChunk);
			// Assert:
			expect(chunkPayload).to.deep.equal(imageBytesToSeparatorIndex);
			expect(chunkText.toString('utf-8')).to.equal(textSection);
		});

		it('can extract payload when only payload present', () => {
			// Arrange:
			const metadata = getMetadata(70);
			const extractChunk = metal.extractChunk(Buffer.from(metadata.value));
			const imageBytesToSeparatorIndex = testData.imageBytes.slice(0, 1012);
			// Act:
			const { chunkPayload, chunkText } = metal.splitChunkPayloadAndText(extractChunk);
			// Assert:
			expect(chunkPayload).to.deep.equal(imageBytesToSeparatorIndex);
			expect(chunkText).to.equal(undefined);
		});
	});

	describe('decode', () => {
		it('can decode binary data from chunks with text', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const firstKey = metadata.scopedMetadataKey;
			const textSection = new MetalSeal(testData.imageBytes.length, 'image/png', 'image.png', 'test').stringify();
			// Act:
			const { payload, text } = metal.decode(firstKey, testData.chunks);
			// Assert:
			expect(payload).to.deep.equal(testData.imageBytes);
			expect(text).to.equal(textSection);
		});

		it('can decode binary data from chunks without text', () => {
			// Arrange:
			const metadata = getMetadata(70);
			const firstKey = metadata.scopedMetadataKey;
			// Act:
			const { payload } = metal.decode(firstKey, testData.chunks);
			// Assert:
			expect(payload).to.deep.equal(testData.imageBytes);
		});

		it('cannot decode binary data chunk is broken', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const firstKey = metadata.scopedMetadataKey;
			// delete chunk
			const deleteChunk = getMetadata(20);
			const deletedChunks = testData.chunks.filter(obj => JSON.stringify(obj.key) !== JSON.stringify(deleteChunk.scopedMetadataKey));
			// Act + Assert:
			expect(() => metal.decode(firstKey, deletedChunks)).to.throw(`the chunk ${deleteChunk.scopedMetadataKey} is missing`);
		});

		it('cannot decode binary data value is broken', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const firstKey = metadata.scopedMetadataKey;
			// delete chunk value
			const mutatedChunk = testData.chunks.find(obj => 20 === obj.id);
			mutatedChunk.value = mutatedChunk.value.slice(0, mutatedChunk.value.length - 1);
			const checksum = metal.generateMetadataKey(new Uint8Array(mutatedChunk.value));
			// Act + Assert:
			expect(() => metal.decode(firstKey, testData.chunks))
				.to.throw(`the chunk ${mutatedChunk.key} is broken (calculated=${checksum})`);
		});
	});

	describe('metal seal', () => {
		const canRoundtripSeal = (seal, hardcodedString) => {
			// Act:
			const sealString = seal.stringify();
			const { isParsed, value } = MetalSeal.tryParse(sealString);
			// Assert:
			expect(isParsed).to.equal(true);
			expect(sealString).to.equal(hardcodedString);
			expect(value.schema).to.deep.equal(seal.schema);
			expect(value.length).to.deep.equal(seal.length);
			expect(value.mimeType).to.deep.equal(seal.mimeType);
			expect(value.name).to.deep.equal(seal.name);
			expect(value.comment).to.deep.equal(seal.comment);
		};
		const cannotRoundtripSeal = json => {
			// Act:
			const { isParsed, value } = MetalSeal.tryParse(json);
			// Assert:
			expect(isParsed).to.equal(false);
			expect(value).to.equal(json);
		};

		it('can roundtrip with all parameters provided', () => {
			// Arrange:
			const seal = new MetalSeal(1, 'image/png', 'image.png', 'test');
			// Act + Assert:
			canRoundtripSeal(seal, '["seal1",1,"image/png","image.png","test"]');
		});

		it('can roundtrip missing comment', () => {
			// Arrange:
			const seal = new MetalSeal(1, 'image/png', 'image.png');
			// Act + Assert:
			canRoundtripSeal(seal, '["seal1",1,"image/png","image.png"]');
		});

		it('can roundtrip missing fileName', () => {
			// Arrange:
			const seal = new MetalSeal(1, 'image/png', undefined, 'test');
			// Act + Assert:
			canRoundtripSeal(seal, '["seal1",1,"image/png",null,"test"]');
		});

		it('can roundtrip missing mimetype and comment', () => {
			// Arrange:
			const seal = new MetalSeal(1, undefined, 'image.png');
			// Act + Assert:
			canRoundtripSeal(seal, '["seal1",1,null,"image.png"]');
		});

		it('cannot roundtrip invalid head', () => {
			// Arrange:
			const failJson = JSON.stringify(['seal1', '1']);
			// Act + Assert:
			cannotRoundtripSeal(failJson);
		});

		it('cannot roundtrip not array', () => {
			// Arrange:
			const failJson = JSON.stringify({ schema: 'seal1', length: 1 });
			// Act + Assert:
			cannotRoundtripSeal(failJson);
		});

		it('cannot roundtrip not compat', () => {
			// Arrange:
			const failJson = JSON.stringify(['seal2', 1]);
			// Act + Assert:
			cannotRoundtripSeal(failJson);
		});
	});
});
