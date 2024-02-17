const { sha3_256 } = require('@noble/hashes/sha3');
const fs = require('fs');

const FILE_PATH = './test/plugins/metadata/resources/metadata.json';
const IMAGE_PATH = './test/plugins/metadata/resources/image.png';
const CHUNK_PAYLOAD_MAX_SIZE = 1012;

const metadataJson = fs.readFileSync(FILE_PATH, 'utf8');
const metadatas = JSON.parse(metadataJson);

const combinePayloadWithText = (payload, text) => {
	const textBytes = text ? Buffer.from(text, 'utf8') : Buffer.alloc(0);
	const textSize = textBytes.length % CHUNK_PAYLOAD_MAX_SIZE
		? textBytes.length + 1 // If extJson section end at mid-chunk then append null char
		: textBytes.length;

	const combinedPayload = new Uint8Array(textSize + payload.length);
	let offset = 0;

	if (textBytes.length) {
		combinedPayload.set(textBytes, offset);
		offset += textBytes.length;
	}
	if (textBytes.length % CHUNK_PAYLOAD_MAX_SIZE) {
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
	const buf = sha3_256(input).buffer;
	const result = new Uint32Array(buf);
	return [result[0], result[1]];
};

const getMetadata = id => metadatas.find(obj => obj.id === id);

const chunks = metadatas.map(obj => ({
	id: obj.id,
	key: obj.scopedMetadataKey,
	value: Buffer.from(obj.value)
}));

const textSection = JSON.stringify({
	MimeType: 'image/png',
	FileName: 'image.png'
});

const imageBytes = fs.readFileSync(IMAGE_PATH);

module.exports = {
	combinePayloadWithText,
	generateChecksum,
	getMetadata,
	metadatas,
	chunks,
	textSection,
	imageBytes
};
