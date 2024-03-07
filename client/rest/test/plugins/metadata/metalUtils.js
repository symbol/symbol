const { sha3_256 } = require('@noble/hashes/sha3');
const fs = require('fs');

const FILE_PATH = `${__dirname}/resources/metadata.json`;
const MOSAIC_FILE_PATH = `${__dirname}/resources/metadata_mosaic.json`;
const IMAGE_PATH = `${__dirname}/resources/image.png`;
const CHUNK_PAYLOAD_MAX_SIZE = 1012;

const combinePayloadWithText = (payload, text) => {
	const textBytes = text ? Buffer.from(text, 'utf8') : Buffer.alloc(0);
	const isEndAtMidChunk = textBytes.length % CHUNK_PAYLOAD_MAX_SIZE;
	const textSize = isEndAtMidChunk ? textBytes.length + 1 : textBytes.length;

	const combinedPayload = new Uint8Array(textSize + payload.length);
	let offset = 0;

	if (textBytes.length) {
		combinedPayload.set(textBytes, offset);
		offset += textBytes.length;
	}
	if (isEndAtMidChunk) {
		// append null char as terminator
		combinedPayload.set([0x00], offset);
		offset++;
	}
	if (payload.length)
		combinedPayload.set(payload, offset);

	return {
		combinedPayload,
		totalTextChunksCount: Math.ceil(textBytes.length / CHUNK_PAYLOAD_MAX_SIZE)
	};
};

const generateChecksum = input => {
	if (0 === input.length)
		throw new Error('Input must not be empty');

	const { buffer } = sha3_256(input);
	const uint32Array = new Uint32Array(buffer);

	return [uint32Array[0], uint32Array[1]];
};

const loadAndFormatMetadata = filePath => {
	const metadataJson = fs.readFileSync(filePath, 'utf8');
	const metadatas = JSON.parse(metadataJson);
	const chunks = metadatas.map(obj => ({
		id: obj.id,
		key: obj.scopedMetadataKey,
		value: Buffer.from(obj.value)
	}));
	return { metadatas, chunks };
};

const { metadatas, chunks } = loadAndFormatMetadata(FILE_PATH);
const { metadatas: mosaicMetadatas, chunks: mosaicChunks } = loadAndFormatMetadata(MOSAIC_FILE_PATH);

const getMetadata = (id, isMosaic = false) => {
	const targetArray = isMosaic ? mosaicMetadatas : metadatas;
	return targetArray.find(obj => obj.id === id);
};

const imageBytes = fs.readFileSync(IMAGE_PATH);

const testData = {
	metadatas,
	mosaicMetadatas,
	chunks,
	mosaicChunks,
	imageBytes
};

module.exports = {
	combinePayloadWithText,
	generateChecksum,
	getMetadata,
	testData
};
