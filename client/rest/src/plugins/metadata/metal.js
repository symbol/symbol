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
		const isValidMetalId = hashHex.startsWith(METAL_ID_HEADER_HEX) && METAL_ID_LENGTH === hashHex.length;

		if (!isValidMetalId)
			throw Error('Invalid metal ID.');

		const compositeHash = hashHex.slice(METAL_ID_HEADER_HEX.length);
		return { compositeHash };
	},

	/**
	 * generate metadata key for checking the metadata is broken or not.
	 * @param {Uint8Array} input value of chunk.
	 * @returns {Uint32Array} Return Metadata key as Uint32Array.
	 */
	generateMetadataKey(input) {
		if (!input.length)
			throw new Error('Input must not be empty');
		const { buffer } = sha3_256(input);
		const uint32Array = new Uint32Array(buffer);
		return [uint32Array[0], uint32Array[1] & 0x7FFFFFFF];
	},

	/**
	 * extract chunk and return the value of a metadata entry according to the metadata protocol.
	 * @param {Buffer} b Value obtained from metadata entry.
	 * @returns {object} Objects containing values in line with the METAL protocol.
	 * - magic: number
	 * - text: boolean
	 * - scopedMetadataKey: uint64
	 * - chunkPayload: Buffer
	 */
	extractChunk(b) {
		const isValidSize = 12 < b.length && 1024 >= b.length;
		if (!isValidSize)
			throw new Error(`Invalid metadata value size ${b.length}`);

		const header = b.subarray(0, 1);
		const magic = header[0] & 0x80;
		const text = Boolean(header[0] & 0x40);

		const scopedMetadataKey = catapult.utils.uint64.fromBytes(new Uint8Array(b.subarray(4, 12)).reverse());
		const chunkPayload = b.subarray(12);

		return {
			magic, text, scopedMetadataKey, chunkPayload
		};
	},

	/**
	 * Separate by null if text is present or not.
	 * @param {object} chunkData chunkPayload and has text or not
	 * - chunkPayload: Buffer
	 * - text: boolean
	 * @returns {object} return chunkPayload and chunkText.
	 * - chunkPayload: Uint8Array
	 * - chunkText: Uint8Array
	 */
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
			const isValidChecksum = 0 === catapult.utils.uint64.compare(checksum, scopedMetadataKey);
			if (!isValidChecksum)
				throw new Error(`Error: The chunk ${scopedMetadataKey} is broken (calculated=${checksum})`);

			const chunkData = this.extractChunk(chunk.value);
			({ magic, scopedMetadataKey } = chunkData);
			const { chunkPayload, chunkText } = this.splitChunkPayloadAndText(chunkData);
			if (chunkPayload.length)
				decodedPayload = Buffer.concat([decodedPayload, chunkPayload]);

			if (chunkText?.length)
				decodedText = Buffer.concat([decodedText, chunkText]);
		} while (0x80 !== (magic & 0x80));
		return {
			payload: decodedPayload,
			text: decodedText.length ? decodedText.toString('utf-8') : undefined
		};
	},

	/**
	 * Get metadata entry by compositehash.
	 * I can't throw exceptions in db tests, so I made it for unit tests.
	 * @param {object} metadata Metadata obtained by composite hash.
	 * @returns {object} metadataEntry
	 */
	getMetadataEntryByCompositehash(metadata) {
		if (0 === metadata.length)
			throw Error('could not get first chunk, it may mistake the metal ID.');
		return metadata[0].metadataEntry;
	}
};

class MetalSeal {
	static SCHEMA = 'seal1';

	static COMPAT = [MetalSeal.SCHEMA];

	/**
	 * @param {number} length size of the file
	 * @param {string} mimeType mime type of the file
	 * @param {string} name file name of the file
	 * @param {string} comment some comment
	 */
	constructor(length, mimeType, name, comment) {
		this.length = length;
		this.mimeType = mimeType;
		this.name = name;
		this.comment = comment;
		this.schema = MetalSeal.SCHEMA;
	}

	static isMetalSealHead(value) {
		return value !== undefined
		&& MetalSeal.SCHEMA === value[0]
		&& 'number' === typeof (value[1])
		&& ('string' === typeof (value[2]) || !value[2])
		&& ('string' === typeof (value[3]) || !value[3])
		&& ('string' === typeof (value[4]) || !value[4]);
	}

	stringify() {
		const result = [
			this.schema,
			this.length
		];

		if (this.mimeType)
			result.push(this.mimeType);
		else if (this.name || this.comment)
			result.push(null);

		if (this.name)
			result.push(this.name);
		else if (this.comment)
			result.push(null);

		if (this.comment)
			result.push(this.comment);

		return JSON.stringify(result);
	}

	static parse(json) {
		try {
			const parsedObject = JSON.parse(json);
			if (!Array.isArray(parsedObject)
				|| !this.isMetalSealHead(parsedObject)
				|| !MetalSeal.COMPAT.includes(parsedObject[0])
			)
				throw new Error('Malformed seal JSON.');

			return new MetalSeal(
				parsedObject[1],
				parsedObject[2] ?? undefined,
				parsedObject[3] ?? undefined,
				parsedObject[4] ?? undefined,
				parsedObject[0]
			);
		} catch {
			return null;
		}
	}
}

module.exports = {
	metal,
	MetalSeal
};
