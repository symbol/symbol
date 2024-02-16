const catapult = require('../../catapult-sdk');
const { sha3_256 } = require('@noble/hashes/sha3');
const bs58 = require('bs58');

const METAL_ID_HEADER_HEX = '0B2A';
const METAL_ID_LENGTH = 68;

const metal = {
	/**
   * convert MetalID to compositehash
	 * @param {string} metalId ex) FeF65JftVPEGwaua35LnbU9jK46uG3W8karGDDuDwVEh8Z.
	 * @returns {string} composite hash.
	 */
	restoreMetadataHash(metalId) {
		const hashHex = catapult.utils.convert.uint8ToHex(bs58.decode(metalId));
		if (!hashHex.startsWith(METAL_ID_HEADER_HEX) || METAL_ID_LENGTH !== hashHex.length)
			throw Error('Invalid metal ID.');
		return { compositeHash: hashHex.slice(METAL_ID_HEADER_HEX.length) };
	},

	/**
	 * generate metadata key for checking the metadata is broken or not.
	 * @param {Buffer} input value of chunk.
	 * @returns {Uint32Array} Return Metadata key as Uint32Array.
	 */
	generateMetadataKey(input) {
		if (0 === input.length)
			throw new Error('Input must not be empty');
		const buf = sha3_256(input).buffer;
		const result = new Uint32Array(buf);
		return [result[0], result[1] & 0x7FFFFFFF];
	},

	/**
	 * extract chunk and return the value of a metadata entry according to the metadata protocol.
	 * @param {Buffer} b Value obtained from metadata entry.
	 * @returns {object} Objects containing values in line with the METAL protocol.
	 * - magic: number
	 * - text: boolean
	 * - scopedMetadataKey: uint64
	 * - chunkPayload: Uint8Array
	 */
	extractChunk(b) {
		if (12 >= b.length || 1024 < b.length)
			throw new Error(`Invalid metadata value size ${b.length}`);
		const header = b.subarray(0, 1);
		const magic = header[0] & 0x80;
		const text = !!(header[0] & 0x40);
		return {
			magic,
			text,
			scopedMetadataKey: catapult.utils.uint64.fromBytes(new Uint8Array(b.subarray(4, 12)).reverse()),
			chunkPayload: b.subarray(12)
		};
	},

	splitChunkPayloadAndText(chunkData) {
		if (!chunkData.text) {
			// No text in the chunk
			return {
				chunkPayload: chunkData.chunkPayload,
				chunkText: undefined
			};
		}

		// Extract text section until null char is encountered.
		const textBytes = [];
		for (let i = 0; i < chunkData.chunkPayload.length && chunkData.chunkPayload[i]; i++)
			textBytes.push(chunkData.chunkPayload[i]);

		return {
			chunkPayload: new Uint8Array(chunkData.chunkPayload.subarray(textBytes.length + 1)),
			chunkText: new Uint8Array(textBytes)
		};
	},

	/**
	 * Decode binary data from chunks.
	 * @param {Uint64} fitstKey ScopedMetadataKey of the beginning chunk.
	 * @param {Array} chunks Chunk containing data to be decoded.
	 * @returns {Buffer} Decoded binary data.
	 */
	decode(fitstKey, chunks) {
		let scopedMetadataKey = fitstKey;
		let magic;
		let decodedPayload = Buffer.from([]);
		let decodedText = Buffer.from([]);
		const findChunk = c => 0 === catapult.utils.uint64.compare(c.key, scopedMetadataKey);
		do {
			const chunk = chunks.find(findChunk);
			if (!chunk)
				throw new Error(`Error: The chunk ${scopedMetadataKey} is missing`);
			const checksum = this.generateMetadataKey(chunk.value);
			if (0 !== catapult.utils.uint64.compare(checksum, scopedMetadataKey))
				throw new Error(`Error: The chunk ${scopedMetadataKey} is broken (calculated=${checksum})`);
			const chunkData = this.extractChunk(chunk.value);
			({ magic, scopedMetadataKey } = chunkData);

			const { chunkPayload, chunkText } = this.splitChunkPayloadAndText(chunkData);

			if (chunkPayload.length) {
				const payloadBuffer = Buffer.alloc(decodedPayload.length + chunkPayload.length);
				payloadBuffer.set(decodedPayload);
				payloadBuffer.set(chunkPayload, decodedPayload.length);
				decodedPayload = payloadBuffer;
			}

			if (chunkText?.length) {
				const textBuffer = Buffer.alloc(decodedText.length + chunkText.length);
				textBuffer.set(decodedText);
				textBuffer.set(chunkText, decodedText.length);
				decodedText = textBuffer;
			}
		} while (0x80 !== (magic & 0x80));
		return {
			payload: decodedPayload,
			text: decodedText.length ? decodedText.toString('utf-8') : undefined
		};
	}
};

module.exports = metal;
