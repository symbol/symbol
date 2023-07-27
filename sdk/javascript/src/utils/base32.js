import charMapping from './charMapping.js';

const ALPHABET = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
const DECODED_BLOCK_SIZE = 5;
const ENCODED_BLOCK_SIZE = 8;

// region encode

const encodeBlock = (input, inputOffset, output, outputOffset) => {
	output[outputOffset + 0] = ALPHABET[input[inputOffset + 0] >> 3];
	output[outputOffset + 1] = ALPHABET[((input[inputOffset + 0] & 0x07) << 2) | (input[inputOffset + 1] >> 6)];
	output[outputOffset + 2] = ALPHABET[(input[inputOffset + 1] & 0x3E) >> 1];
	output[outputOffset + 3] = ALPHABET[((input[inputOffset + 1] & 0x01) << 4) | (input[inputOffset + 2] >> 4)];
	output[outputOffset + 4] = ALPHABET[((input[inputOffset + 2] & 0x0F) << 1) | (input[inputOffset + 3] >> 7)];
	output[outputOffset + 5] = ALPHABET[(input[inputOffset + 3] & 0x7F) >> 2];
	output[outputOffset + 6] = ALPHABET[((input[inputOffset + 3] & 0x03) << 3) | (input[inputOffset + 4] >> 5)];
	output[outputOffset + 7] = ALPHABET[input[inputOffset + 4] & 0x1F];
};

// endregion

// region decode

const Char_To_Decoded_Char_Map = (() => {
	const builder = charMapping.createBuilder();
	builder.addRange('A', 'Z', 0);
	builder.addRange('2', '7', 26);
	return builder.map;
})();

const decodeChar = c => {
	const decodedChar = Char_To_Decoded_Char_Map[c];
	if (undefined !== decodedChar)
		return decodedChar;

	throw Error(`illegal base32 character ${c}`);
};

const decodeBlock = (input, inputOffset, output, outputOffset) => {
	const bytes = new Uint8Array(ENCODED_BLOCK_SIZE);
	for (let i = 0; i < ENCODED_BLOCK_SIZE; ++i)
		bytes[i] = decodeChar(input[inputOffset + i]);

	output[outputOffset + 0] = (bytes[0] << 3) | (bytes[1] >> 2);
	output[outputOffset + 1] = ((bytes[1] & 0x03) << 6) | (bytes[2] << 1) | (bytes[3] >> 4);
	output[outputOffset + 2] = ((bytes[3] & 0x0F) << 4) | (bytes[4] >> 1);
	output[outputOffset + 3] = ((bytes[4] & 0x01) << 7) | (bytes[5] << 2) | (bytes[6] >> 3);
	output[outputOffset + 4] = ((bytes[6] & 0x07) << 5) | bytes[7];
};

// endregion

/**
 * Base32 encodes a binary buffer.
 * @param {Uint8Array} data Binary data to encode.
 * @returns {string} Base32 encoded string corresponding to the input data.
 */
const encode = data => {
	if (0 !== data.length % DECODED_BLOCK_SIZE)
		throw Error(`decoded size must be multiple of ${DECODED_BLOCK_SIZE}`);

	const output = new Array(data.length / DECODED_BLOCK_SIZE * ENCODED_BLOCK_SIZE);
	for (let i = 0; i < data.length / DECODED_BLOCK_SIZE; ++i)
		encodeBlock(data, i * DECODED_BLOCK_SIZE, output, i * ENCODED_BLOCK_SIZE);

	return output.join('');
};

/**
 * Base32 decodes a base32 encoded string.
 * @param {string} encoded Base32 encoded string to decode.
 * @returns {Uint8Array} Binary data corresponding to the input string.
 */
const decode = encoded => {
	if (0 !== encoded.length % ENCODED_BLOCK_SIZE)
		throw Error(`encoded size must be multiple of ${ENCODED_BLOCK_SIZE}`);

	const output = new Uint8Array(encoded.length / ENCODED_BLOCK_SIZE * DECODED_BLOCK_SIZE);
	for (let i = 0; i < encoded.length / ENCODED_BLOCK_SIZE; ++i)
		decodeBlock(encoded, i * ENCODED_BLOCK_SIZE, output, i * DECODED_BLOCK_SIZE);

	return output;
};

export default { encode, decode };
