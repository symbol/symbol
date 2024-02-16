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

const metal = require('../../../src/plugins/metadata/metal');
const { expect } = require('chai');
const fs = require('fs');

const chunksValueArrayToBuffer = metadataJson => {
	const chunks = JSON.parse(metadataJson);
	chunks.forEach(chunk => {
		if (Array.isArray(chunk.value))
			chunk.value = Buffer.from(chunk.value);
	});
	return chunks;
};

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
		it('extract chunk and returns the value with chunk', () => {
			// Arrange:
			const b = Buffer.from([0x00, 0x31, 0x00, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0A, 0x0B]);
			// Act:
			const result = metal.extractChunk(b);
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x00,
				scopedMetadataKey: [0x08090A0B, 0x04050607],
				chunkPayload: Buffer.from([0x0A, 0x0B]),
				text: false
			});
		});
		it('extract chunk and returns the value with end chunk', () => {
			// Arrange:
			const b = Buffer.from([0x80, 0x31, 0x00, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0A, 0x0B]);
			// Act:
			const result = metal.extractChunk(b);
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x80,
				scopedMetadataKey: [0x08090A0B, 0x04050607],
				chunkPayload: Buffer.from([0x0A, 0x0B]),
				text: false
			});
		});
		it('splits and returns the value with text', () => {
			// Arrange:
			const b = Buffer.from([0x40, 0x31, 0x00, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0A, 0x00, 0x0B]);
			// Act:
			const result = metal.extractChunk(b);
			// Assert:
			expect(result).to.deep.equal({
				magic: 0x00,
				scopedMetadataKey: [0x08090A0B, 0x04050607],
				chunkPayload: Buffer.from([0x0A, 0x00, 0x0B]),
				text: true
			});
		});
		it('can not splits size is shorten', () => {
			// Arrange:
			const b = Buffer.from([0x80, 0x31, 0x00, 0x00, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A]);
			// Act:
			const result = () => metal.extractChunk(b);
			// Assert:
			expect(result).to.throw(`Invalid metadata value size ${b.length}`);
		});
		it('can not splits size is longer', () => {
			// Arrange:
			const b = Buffer.alloc(1025, 0xFF);
			// Act:
			const result = () => metal.extractChunk(b);
			// Assert:
			expect(result).to.throw(`Invalid metadata value size ${b.length}`);
		});
	});
	describe('generateMetadataKey', () => {
		it('generate metadata key', () => {
			// Arrange:
			const input = Buffer.from([0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B]);
			// Act:
			const result = metal.generateMetadataKey(input);
			// Assert:
			expect(result).to.deep.equal([0x2DD9CB4A, 0x06C30F31]);
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
			const metadataJson = fs.readFileSync('./test/plugins/metadata/metadata.json', 'utf8');
			const chunks = chunksValueArrayToBuffer(metadataJson);
			const textChunkKey = [1636421861, 1538397383];
			const exceptedText = {
				MimeType: 'image/png',
				FileName: 'image.png'
			};
			const textChunk = chunks.find(obj => JSON.stringify(obj.key) === JSON.stringify(textChunkKey));
			const extractChunk = metal.extractChunk(textChunk.value);

			// Act:
			const { chunkText } = metal.splitChunkPayloadAndText(extractChunk);
			// Assert:
			expect(Buffer.from(chunkText.buffer).toString('utf-8')).to.equal(JSON.stringify(exceptedText));
		});
		it('split chunk payload and text but does not have text', () => {
			// Arrange:
			const metadataJson = fs.readFileSync('./test/plugins/metadata/metadata.json', 'utf8');
			const chunks = chunksValueArrayToBuffer(metadataJson);
			const nonTextChunkKey = [242152552, 118238590];
			const nonTextChunk = chunks.find(obj => JSON.stringify(obj.key) === JSON.stringify(nonTextChunkKey));
			const extractChunk = metal.extractChunk(nonTextChunk.value);

			// Act:
			const { chunkText } = metal.splitChunkPayloadAndText(extractChunk);
			// Assert:
			expect(chunkText).to.equal(undefined);
		});
	});
	describe('decode', () => {
		it('decodes binary data from chunks with text', () => {
			// Arrange:
			const fitstKey = [1636421861, 1538397383];
			const metadataJson = fs.readFileSync('./test/plugins/metadata/metadata.json', 'utf8');
			const chunks = chunksValueArrayToBuffer(metadataJson);
			const image = fs.readFileSync('./test/plugins/metadata/image.png');
			const exceptedText = {
				MimeType: 'image/png',
				FileName: 'image.png'
			};
			// Act:
			const { payload, text } = metal.decode(fitstKey, chunks);
			// Assert:
			expect(payload).to.deep.equal(image);
			expect(text).to.equal(JSON.stringify(exceptedText));
		});
		it('decodes binary data from chunks without text', () => {
			// Arrange:
			const fitstKey = [986594817, 1604531706];
			const metadataJson = fs.readFileSync('./test/plugins/metadata/metadata.json', 'utf8');
			const chunks = chunksValueArrayToBuffer(metadataJson);
			const image = fs.readFileSync('./test/plugins/metadata/image.png');
			// Act:
			const { payload } = metal.decode(fitstKey, chunks);
			// Assert:
			expect(payload).to.deep.equal(image);
		});
		it('can not decodes binary data chunk is broken', () => {
			// Arrange:
			const fitstKey = [986594817, 1604531706];
			const metadataJson = fs.readFileSync('./test/plugins/metadata/metadata.json', 'utf8');
			const chunks = chunksValueArrayToBuffer(metadataJson);
			const deleteChunkKey = [892070002, 409383513];
			// delete chunk
			const deletedChunks = chunks.filter(obj => JSON.stringify(obj.key) !== JSON.stringify(deleteChunkKey));

			// Act:
			const result = () => metal.decode(fitstKey, deletedChunks);
			// Assert:
			expect(result).to.throw(`Error: The chunk ${deleteChunkKey} is missing`);
		});
		it('can not decodes binary data value is broken', () => {
			// Arrange:
			const fitstKey = [986594817, 1604531706];
			const metadataJson = fs.readFileSync('./test/plugins/metadata/metadata.json', 'utf8');
			const chunks = chunksValueArrayToBuffer(metadataJson);
			const deleteValueChunkKey = [892070002, 409383513];
			const deleteValueChunk = chunks.find(chunk => JSON.stringify(chunk.key) === JSON.stringify(deleteValueChunkKey));
			// delete last value
			deleteValueChunk.value = deleteValueChunk.value.slice(0, deleteValueChunk.value.length - 1);

			const checksum = metal.generateMetadataKey(deleteValueChunk.value);
			// Act:
			const result = () => metal.decode(fitstKey, chunks);
			// Assert:
			expect(result).to.throw(`Error: The chunk ${deleteValueChunkKey} is broken (calculated=${checksum})`);
		});
	});
});
