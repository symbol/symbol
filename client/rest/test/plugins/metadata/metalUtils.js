import { sha3_256 } from '@noble/hashes/sha3';
import { utils } from 'symbol-sdk';
import fs from 'fs';

const FILE_PATH = `${import.meta.dirname}/resources/metadata.json`;
const MOSAIC_FILE_PATH = `${import.meta.dirname}/resources/metadata_mosaic.json`;
const IMAGE_PATH = `${import.meta.dirname}/resources/image.png`;
const CHUNK_PAYLOAD_MAX_SIZE = 1012;

export const combinePayloadWithText = (payload, text) => {
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

export const generateChecksum = input => {
	if (0 === input.length)
		throw new Error('Input must not be empty');

	const hash = sha3_256(input);
	return utils.bytesToBigInt(hash, 8);
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

export const getMetadata = (id, isMosaic = false) => {
	const targetArray = isMosaic ? mosaicMetadatas : metadatas;
	return targetArray.find(obj => obj.id === id);
};

const imageBytes = fs.readFileSync(IMAGE_PATH);

export const testData = {
	metadatas,
	mosaicMetadatas,
	chunks,
	mosaicChunks,
	imageBytes
};
