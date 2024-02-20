const { sha3_256 } = require('@noble/hashes/sha3');
const fs = require('fs');

const FILE_PATH = './test/plugins/metadata/resources/metadata.json';
const MOSAIC_FILE_PATH = './test/plugins/metadata/resources/metadata_mosaic.json';
const IMAGE_PATH = './test/plugins/metadata/resources/image.png';
const CHUNK_PAYLOAD_MAX_SIZE = 1012;

const metadataJson = fs.readFileSync(FILE_PATH, 'utf8');
const mosaicMetadataJson = fs.readFileSync(MOSAIC_FILE_PATH, 'utf8');
const metadatas = JSON.parse(metadataJson);
const mosaicMetadatas = JSON.parse(mosaicMetadataJson);

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
		textChunks: Math.ceil(textBytes.length / CHUNK_PAYLOAD_MAX_SIZE)
	};
};

const generateChecksum = input => {
	if (0 === input.length)
		throw new Error('Input must not be empty');

	const { buffer } = sha3_256(input);
	const uint32Array = new Uint32Array(buffer);

	return [uint32Array[0], uint32Array[1]];
};

const getMetadata = (id, isMosaic = false) => {
	if (!isMosaic)
		return metadatas.find(obj => obj.id === id);
	return mosaicMetadatas.find(obj => obj.id === id);
};

const chunks = metadatas.map(obj => ({
	id: obj.id,
	key: obj.scopedMetadataKey,
	value: Buffer.from(obj.value)
}));

const moasicChunks = mosaicMetadatas.map(obj => ({
	id: obj.id,
	key: obj.scopedMetadataKey,
	value: Buffer.from(obj.value)
}));

const imageBytes = fs.readFileSync(IMAGE_PATH);

module.exports = {
	combinePayloadWithText,
	generateChecksum,
	getMetadata,
	metadatas,
	mosaicMetadatas,
	chunks,
	moasicChunks,
	imageBytes
};
