const catapult = require('../../catapult-sdk');
const { sha3_256 } = require('@noble/hashes/sha3');
const bs58 = require('bs58');

const METAL_ID_HEADER_SIGNATURE = [0x0B, 0x2A];
const METAL_ID_LENGTH = 34;

const metal = {
	/**
	 * Convert a metal id (e.g. 'FeF65JftVPEGwaua35LnbU9jK46uG3W8karGDDuDwVEh8Z') into its associated composite hash.
	 * @param {string} metalId Metal id to convert.
	 * @returns {Uint8Array} Associated composite hash.
	 */
	extractCompositeHashFromMetalId(metalId) {
		const decodedBs58HashBytes = bs58.decode(metalId);
		const isValidMetalId = METAL_ID_HEADER_SIGNATURE[0] === decodedBs58HashBytes[0]
			&& METAL_ID_HEADER_SIGNATURE[1] === decodedBs58HashBytes[1]
			&& METAL_ID_LENGTH === decodedBs58HashBytes.length;

		if (!isValidMetalId)
			throw Error(`'${metalId}' is not a valid metal id`);

		return decodedBs58HashBytes
			.subarray(METAL_ID_HEADER_SIGNATURE.length);
	},

	/**
	 * Generates a metadata key to check if the metadata is broken or not.
	 * @param {Uint8Array} input Chunk value to generate the key from.
	 * @returns {module:utils/uint64} Metadata key as a uint64, represented as an array of two numbers.
	 */
	generateMetadataKey(input) {
		if (!input.length)
			throw new Error('input must not be empty');
		const hashedBytes = sha3_256(input);
		const uint64 = catapult.utils.uint64.fromBytes(hashedBytes.subarray(0, 8));
		return [uint64[0], uint64[1] & 0x7FFFFFFF];
	},

	/**
	 * Extract chunk and return the value of a metadata entry according to the metadata protocol.
	 * @param {Buffer} buffer Value obtained from metadata entry.
	 * @returns {object} Object containing values in line with the METAL protocol.
	 * - magic: number - 0x00 is chunk, 0x80 is end of chunk.
	 * - text: boolean - True if text is present, false otherwise.
	 * - scopedMetadataKey: uint64 - ScopedMetadataKey of the next chunk.
	 * - chunkPayload: Buffer - Payload of the chunk.
	 */
	extractChunk(buffer) {
		const header = buffer.subarray(0, 1);
		const magic = header[0] & 0x80;
		const text = Boolean(header[0] & 0x40);

		const scopedMetadataKey = catapult.utils.uint64.fromBytes(new Uint8Array(buffer.subarray(4, 12)).reverse());
		const chunkPayload = buffer.subarray(12);

		return {
			magic, text, scopedMetadataKey, chunkPayload
		};
	},

	/**
	 * Separate by null if text is present or not.
	 * @param {object} chunkData ChunkPayload and has text or not
	 * - chunkPayload: Buffer - Payload of chunk that may contain text
	 * - text: boolean - true if text is present, false otherwise.
	 * @returns {object} Return chunkPayload and chunkText.
	 * - chunkPayload: Buffer - If text is present, separated chunkPayload. otherwise, original chunkPayload.
	 * - chunkText: Buffer - If text is present, separated chunkText. otherwise, undefined.
	 */
	splitChunkPayloadAndText(chunkData) {
		if (!chunkData.text) {
			// No text in the chunk
			return {
				chunkPayload: chunkData.chunkPayload,
				chunkText: undefined
			};
		}

		// Find the position of the null char.
		const separatorIndex = chunkData.chunkPayload.indexOf(0);

		return {
			chunkPayload: chunkData.chunkPayload.subarray(separatorIndex + 1),
			chunkText: chunkData.chunkPayload.subarray(0, separatorIndex)
		};
	},

	/**
	 * Decode binary data from chunks.
	 * @param {module:utils/uint64~uint64} firstKey ScopedMetadataKey of the beginning chunk.
	 * @param {Array} chunks Chunk containing data to be decoded.
	 * @returns {Buffer} Decoded binary data.
	 */
	decode(firstKey, chunks) {
		let scopedMetadataKey = firstKey;
		let magic;
		const decodedPayloads = [];
		const decodedTexts = [];
		const findChunk = chunk => 0 === catapult.utils.uint64.compare(chunk.key, scopedMetadataKey);

		do {
			const chunk = chunks.find(findChunk);
			if (!chunk)
				throw new Error(`the chunk ${scopedMetadataKey} is missing`);

			const metadataKey = this.generateMetadataKey(chunk.value);
			const isValidMetadataKey = 0 === catapult.utils.uint64.compare(metadataKey, scopedMetadataKey);
			if (!isValidMetadataKey)
				throw new Error(`the chunk ${scopedMetadataKey} is broken (calculated=${metadataKey})`);

			const chunkData = this.extractChunk(chunk.value);
			({ magic, scopedMetadataKey } = chunkData);
			const { chunkPayload, chunkText } = this.splitChunkPayloadAndText(chunkData);
			if (chunkPayload.length)
				decodedPayloads.push(chunkPayload);

			if (chunkText?.length)
				decodedTexts.push(chunkText);
		} while (0x80 !== magic);
		return {
			payload: Buffer.concat(decodedPayloads),
			text: decodedTexts.length ? Buffer.concat(decodedTexts).toString('utf-8') : undefined
		};
	}
};

/**
 * MetalSeal is a simple JSON schema for writing file informations
 * filellength, mimetype, filename and comment in text sections.
 */
class MetalSeal {
	static SCHEMA = 'seal1';

	static COMPAT = [MetalSeal.SCHEMA];

	/**
	 * Creates a metal seal object from provided arguments.
	 * @param {number} length Size of the file
	 * @param {string} mimeType Mime type of the file
	 * @param {string} name File name of the file
	 * @param {string} comment Some comment
	 */
	constructor(length, mimeType, name, comment) {
		this.length = length;
		this.mimeType = mimeType;
		this.name = name;
		this.comment = comment;
		this.schema = MetalSeal.SCHEMA;
	}

	/**
	 * Checks if the given value is a MetalSeal head.
	 * @param {any} value - The value to check
	 * @returns {boolean} - Returns true if the value is a MetalSeal head, false otherwise
	 */
	static isHead(value) {
		return value !== undefined
		&& MetalSeal.SCHEMA === value[0]
		&& 'number' === typeof (value[1])
		&& ('string' === typeof (value[2]) || !value[2])
		&& ('string' === typeof (value[3]) || !value[3])
		&& ('string' === typeof (value[4]) || !value[4]);
	}

	/**
	 * Converts the MetalSeal object into a JSON string.
	 * The resulting string includes the schema, length, mimeType (if exists), name (if exists), and comment (if exists).
	 * @returns {string} - A JSON string representation of the MetalSeal object.
	 */
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

	/**
	 * Try parses a JSON string into a MetalSeal object.
	 * The JSON string should represent an array with the schema, length, mimeType (if exists), name (if exists), and comment (if exists).
	 * @param {string} json - The JSON string to parse.
	 * @returns {object} - If could parse, return isParsed is true and a new MetalSeal object created from the parsed JSON string.
	 * Otherwise, isParsed is false and the return value is original.
	 * - isParsed: boolean Could parse or not
	 * - value: MetalSeal | string If isParsed is true MetalSeal, otherwise original JSON string
	 */
	static tryParse(json) {
		let parsedObject;
		try {
			parsedObject = JSON.parse(json);
		} catch {
			return { isParsed: false, value: json };
		}
		if (!Array.isArray(parsedObject)
			|| !this.isHead(parsedObject)
			|| !MetalSeal.COMPAT.includes(parsedObject[0])
		)
			return { isParsed: false, value: json };

		return {
			isParsed: true,
			value: new MetalSeal(
				parsedObject[1],
				parsedObject[2] ?? undefined,
				parsedObject[3] ?? undefined,
				parsedObject[4] ?? undefined
			)
		};
	}
}

module.exports = {
	metal,
	MetalSeal
};
