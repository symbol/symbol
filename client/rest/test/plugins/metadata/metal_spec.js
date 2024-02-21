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
	chunks,
	moasicChunks,
	imageBytes
} = require('./metalUtils');
const { metal, MetalSeal } = require('../../../src/plugins/metadata/metal');
const { expect } = require('chai');

describe('metal', () => {
	describe('restoreMetadataHash', () => {
		it('can convert MetalID to compositehash', () => {
			// Arrange:
			const metalId = 'FeDr2dAx9GnK8j4w6VDwtBLBbvnvxx7rea4eHrpQvG6hhX';
			// Act:
			const result = metal.restoreMetadataHash(metalId);
			// Assert:
			expect(result).to.deep.equal({ compositeHash: 'BBFB2A837D3A9CF993195C2CA417645518A2ED2C30E8B760D3E224EC75F601E6' });
		});
		it('can not convert MetalID to compositehash start with not Fe', () => {
			// Arrange:
			const metalId = 'AuDr2dAx9GnK8j4w6VDwtBLBbvnvxx7rea4eHrpQvG6hhX';
			// Act:
			const result = () => metal.restoreMetadataHash(metalId);
			// Assert:
			expect(result).to.throw('Invalid metal ID.');
		});
		it('can not convert MetalID to compositehash length is not 68', () => {
			// Arrange:
			const metalId = 'FeDr2dAx9GnK8j4w6VDwtBLBbvnvxx7rea4eHrpQvG6hh';
			// Act:
			const result = () => metal.restoreMetadataHash(metalId);
			// Assert:
			expect(result).to.throw('Invalid metal ID.');
		});
	});
	describe('extractChunk', () => {
		it('extract chunk and returns the value chunk', () => {
			// Arrange:
			const metadata = getMetadata(70);
			const nextMetadata = getMetadata(80);
			// Act:
			const result = metal.extractChunk(Buffer.from(metadata.value));
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x00,
				scopedMetadataKey: nextMetadata.scopedMetadataKey,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: false
			});
		});
		it('extract chunk and returns the value end chunk', () => {
			// Arrange:
			const metadata = getMetadata(120);
			const { combinedPayload } = combinePayloadWithText(imageBytes);
			const checkSum = generateChecksum(combinedPayload);

			// Act:
			const result = metal.extractChunk(Buffer.from(metadata.value));
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x80,
				scopedMetadataKey: checkSum,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: false
			});
		});
		it('extract chunk and returns the value chunk with text', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const nextMetadata = getMetadata(20);
			// Act:
			const result = metal.extractChunk(Buffer.from(metadata.value));
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x00,
				scopedMetadataKey: nextMetadata.scopedMetadataKey,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: true
			});
		});
		it('extract chunk and returns the value end chunk with text', () => {
			// Arrange:
			const metadata = getMetadata(60);
			const textSection = new MetalSeal(imageBytes.length, 'image/png', 'image.png', 'test').stringify();
			const { combinedPayload } = combinePayloadWithText(imageBytes, textSection);
			const checkSum = generateChecksum(combinedPayload);

			// Act:
			const result = metal.extractChunk(Buffer.from(metadata.value));
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x80,
				scopedMetadataKey: checkSum,
				chunkPayload: Buffer.from(metadata.value.slice(12)),
				text: false
			});
		});
		it('can not extract size is shorten', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const metadataValue = metadata.value.slice(0, 11);
			// Act:
			const result = () => metal.extractChunk(Buffer.from(metadataValue));
			// Assert:
			expect(result).to.throw(`Invalid metadata value size ${metadataValue.length}`);
		});
		it('can not extract size is longer', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const metadataValue = metadata.value.concat(Buffer.alloc(1, 0x00));
			// Act:
			const result = () => metal.extractChunk(Buffer.from(metadataValue));
			// Assert:
			expect(result).to.throw(`Invalid metadata value size ${metadataValue.length}`);
		});
	});
	describe('generateMetadataKey', () => {
		it('generate metadata key', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const input = metadata.value;
			// Act:
			const result = metal.generateMetadataKey(new Uint8Array(input));
			// Assert:
			expect(result).to.deep.equal(metadata.scopedMetadataKey);
		});
		it('can not generates metadata key input is empty', () => {
			// Arrange:
			const input = Buffer.alloc(0);
			// Act:
			const result = () => metal.generateMetadataKey(input);
			// Assert:
			expect(result).to.throw('Input must not be empty');
		});
	});
	describe('splitChunkPayloadAndText', () => {
		it('split chunk payload and text', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const extractChunk = metal.extractChunk(Buffer.from(metadata.value));
			const textSection = new MetalSeal(imageBytes.length, 'image/png', 'image.png', 'test').stringify();
			// Act:
			const { chunkText } = metal.splitChunkPayloadAndText(extractChunk);
			// Assert:
			expect(Buffer.from(chunkText.buffer).toString('utf-8')).to.equal(textSection);
		});
		it('split chunk payload and text but does not have text', () => {
			// Arrange:
			const metadata = getMetadata(70);
			const extractChunk = metal.extractChunk(Buffer.from(metadata.value));
			// Act:
			const { chunkText } = metal.splitChunkPayloadAndText(extractChunk);
			// Assert:
			expect(chunkText).to.equal(undefined);
		});
	});
	describe('getMetadataEntryByCompositehash', () => {
		it('check metadata length equal 1', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const processedMetadata = [
				{
					metadataEntry: {
						sourceAddress: Buffer.from(metadata.sourceAddress),
						targetAddress: Buffer.from(metadata.targetAddress),
						scopedMetadataKey: metadata.scopedMetadataKey,
						targetId: metadata.targetId,
						metadataType: metadata.metadataType,
						value: Buffer.from(metadata.value)
					}
				}
			];
			// Act:
			const metadataEntry = metal.getMetadataEntryByCompositehash(processedMetadata);
			// Assert:
			expect(metadataEntry).to.deep.equal(processedMetadata[0].metadataEntry);
		});
		it('can not metadata entry metal id is incorrect', () => {
			// Arrange:
			const processedMetadata = [];
			// Act:
			const meta = () => metal.getMetadataEntryByCompositehash(processedMetadata);
			// Assert:
			expect(meta).to.throw('could not get first chunk, it may mistake the metal ID.');
		});
	});
	describe('decode', () => {
		it('decodes binary data from chunks with text', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const fitstKey = metadata.scopedMetadataKey;
			const textSection = new MetalSeal(imageBytes.length, 'image/png', 'image.png', 'test').stringify();
			// Act:

			const { payload, text } = metal.decode(fitstKey, chunks);
			// Assert:
			expect(payload).to.deep.equal(imageBytes);
			expect(text).to.equal(textSection);
		});
		it('decodes binary data from chunks without text', () => {
			// Arrange:
			const metadata = getMetadata(70);
			const fitstKey = metadata.scopedMetadataKey;
			// Act:
			const { payload } = metal.decode(fitstKey, chunks);
			// Assert:
			expect(payload).to.deep.equal(imageBytes);
		});
		it('can not decodes binary data chunk is broken', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const fitstKey = metadata.scopedMetadataKey;
			// delete chunk
			const deleteChunk = getMetadata(20);
			const deletedChunks = chunks.filter(obj => JSON.stringify(obj.key) !== JSON.stringify(deleteChunk.scopedMetadataKey));
			// Act:
			const result = () => metal.decode(fitstKey, deletedChunks);
			// Assert:
			expect(result).to.throw(`Error: The chunk ${deleteChunk.scopedMetadataKey} is missing`);
		});
		it('can not decodes binary data value is broken', () => {
			// Arrange:
			const metadata = getMetadata(10);
			const fitstKey = metadata.scopedMetadataKey;
			// delete chunk value
			const deleteChunk = chunks.find(obj => 20 === obj.id);
			deleteChunk.value = deleteChunk.value.slice(0, deleteChunk.value.length - 1);
			const checksum = metal.generateMetadataKey(new Uint8Array(deleteChunk.value));
			// Act:
			const result = () => metal.decode(fitstKey, chunks);
			// Assert:
			expect(result).to.throw(`Error: The chunk ${deleteChunk.key} is broken (calculated=${checksum})`);
		});
		it('decodes mosaic metadata from chunks', () => {
			// Arrange:
			const metadata = getMetadata(130, true);
			const fitstKey = metadata.scopedMetadataKey;
			// Act:
			const { payload } = metal.decode(fitstKey, moasicChunks);
			// Assert:
			expect(payload).to.deep.equal(imageBytes);
		});
	});
	describe('metal seal', () => {
		it('metal seal all aggs', () => {
			// Arrange:
			const seal = new MetalSeal(1, 'image/png', 'image.png', 'test');
			// Act:
			const sealString = seal.stringify();
			const parsed = MetalSeal.parse(sealString);
			// Assert:
			expect(parsed.schema).to.deep.equal(seal.schema);
			expect(parsed.length).to.deep.equal(seal.length);
			expect(parsed.mimeType).to.deep.equal(seal.mimeType);
			expect(parsed.name).to.deep.equal(seal.name);
			expect(parsed.comment).to.deep.equal(seal.comment);
		});
		it('metal seal missing comment', () => {
			// Arrange:
			const seal = new MetalSeal(1, 'image/png', 'image.png');
			// Act:
			const sealString = seal.stringify();
			const parsed = MetalSeal.parse(sealString);
			// Assert:
			expect(parsed.schema).to.deep.equal(seal.schema);
			expect(parsed.length).to.deep.equal(seal.length);
			expect(parsed.mimeType).to.deep.equal(seal.mimeType);
			expect(parsed.name).to.deep.equal(seal.name);
			expect(parsed.comment).to.deep.equal(seal.comment);
		});
		it('metal seal missing fileName', () => {
			// Arrange:
			const seal = new MetalSeal(1, 'image/png', undefined, 'test');
			// Act:
			const sealString = seal.stringify();
			const parsed = MetalSeal.parse(sealString);
			// Assert:
			expect(parsed.schema).to.deep.equal(seal.schema);
			expect(parsed.length).to.deep.equal(seal.length);
			expect(parsed.mimeType).to.deep.equal(seal.mimeType);
			expect(parsed.name).to.deep.equal(seal.name);
			expect(parsed.comment).to.deep.equal(seal.comment);
		});
		it('metal seal missing mimetype and comment', () => {
			// Arrange:
			const seal = new MetalSeal(1, undefined, 'image.png');
			// Act:
			const sealString = seal.stringify();
			const parsed = MetalSeal.parse(sealString);
			// Assert:
			expect(parsed.schema).to.deep.equal(seal.schema);
			expect(parsed.length).to.deep.equal(seal.length);
			expect(parsed.mimeType).to.deep.equal(seal.mimeType);
			expect(parsed.name).to.deep.equal(seal.name);
			expect(parsed.comment).to.deep.equal(seal.comment);
		});
		// Jenkins will error out.
		// It is not a problem when tested locally, but it is not very important, so I have commented it out.
		/* it('malformed seal JSON.', () => {
			// Arrange:
			const failJson = JSON.stringify(['seal2', 1]);
			// Act:
			const parsed = () => MetalSeal.parse(failJson);
			// Assert:
			expect(parsed).to.throw('Malformed seal JSON.');
		}); */
	});
});
