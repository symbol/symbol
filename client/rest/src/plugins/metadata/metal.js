const catapult = require('../../catapult-sdk');
const { longToUint64 } = require('../../db/dbUtils');
const { sha3_256 } = require('@noble/hashes/sha3');
const bs58 = require('bs58');

const METAL_ID_HEADER_HEX = '0B2A';

const metal = {
	/**
   * convert MetalID to compositehash
	 * @param {string} metalId ex) FeF65JftVPEGwaua35LnbU9jK46uG3W8karGDDuDwVEh8Z.
	 * @returns {string} composite hash.
	 */
	restoreMetadataHash(metalId) {
		const hashHex = catapult.utils.convert.uint8ToHex(bs58.decode(metalId));
		if (!hashHex.startsWith(METAL_ID_HEADER_HEX))
			throw Error('Invalid metal ID.');
		return { compositeHash: hashHex.slice(METAL_ID_HEADER_HEX.length) };
	},

	/**
   * Split and return the value of a metadata entry according to the metadata protocol.
   * @param {Buffer} b Value obtained from metadata entry.
   * @returns {object} Objects containing values in line with the METAL protocol.
   */
	splitBuffer(b) {
		return {
			magic: b.subarray(0, 1),
			scopedMetadataKey: catapult.utils.uint64.fromBytes(new Uint8Array(b.subarray(4, 12)).reverse()),
			value: b.subarray(12)
		};
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
	 * Decode binary data from chunks.
	 * @param {Uint64} fitstKey ScopedMetadataKey of the beginning chunk.
	 * @param {Array} chunks Chunk containing data to be decoded.
	 * @returns {Buffer} Decoded binary data.
	 */
	decodeData(fitstKey, chunks) {
		let scopedMetadataKey = fitstKey;
		let magic;
		let value;
		let valueResult = Buffer.from([]);
		const findChunk = c => 0 === catapult.utils.uint64.compare(c.key, scopedMetadataKey);
		do {
			const chunk = chunks.find(findChunk);
			if (!chunk)
				throw new Error(`Error: The chunk ${scopedMetadataKey} is missing`);
			const checksum = this.generateMetadataKey(chunk.value);
			if (0 !== catapult.utils.uint64.compare(checksum, scopedMetadataKey))
				throw new Error(`Error: The chunk ${scopedMetadataKey} is broken (calculated=${checksum})`);
			({ scopedMetadataKey, magic, value } = this.splitBuffer(chunk.value));
			valueResult = Buffer.concat([valueResult, value]);
		} while (0x80 !== (magic[0] & 0x80));
		return valueResult;
	},

	/**
	 * Read and decode ScopedMetadaKey in order, usually not used because it is slow.
	 * @param {MetadataDb} db database
	 * @param {Uint8Array} compositeHashes metal Id
	 * @returns {Buffer} Decoded binary data.
	 */
	async decodeDataStepByStep(db, compositeHashes) {
		const chunks = [];
		const firstChunk = (await db.metadatasByCompositeHash(compositeHashes))[0];
		if (!firstChunk)
			throw Error('could not get first chunk, it may mistake the metal ID.');
		chunks.push({
			key: longToUint64(firstChunk.metadataEntry.scopedMetadataKey),
			value: firstChunk.metadataEntry.value.buffer
		});
		const { metadataEntry } = firstChunk;
		const { magic, scopedMetadataKey } = this.splitBuffer(metadataEntry.value.buffer);

		const fetchMetadata = async (_magic, _scopedMetadataKey) => {
			let magicValue = _magic;
			let scopedMetadataKeyValue = _scopedMetadataKey;
			const options = {
				sortField: 'id', sortDirection: 1, pageSize: 1, pageNumber: 1
			};
			const chunk = await db.metadata(
				new Uint8Array(metadataEntry.sourceAddress.buffer),
				new Uint8Array(metadataEntry.targetAddress.buffer),
				scopedMetadataKeyValue,
				metadataEntry.targetId,
				metadataEntry.metadataType,
				options
			);
			chunks.push({
				key: longToUint64(chunk.data[0].metadataEntry.scopedMetadataKey),
				value: chunk.data[0].metadataEntry.value.buffer
			});
			const splited = this.splitBuffer(chunk.data[0].metadataEntry.value.buffer);
			magicValue = splited.magic;
			scopedMetadataKeyValue = splited.scopedMetadataKey;
			if (0x80 !== (magicValue[0] & 0x80))
				await fetchMetadata(magicValue, scopedMetadataKeyValue);
		};
		await fetchMetadata(magic, scopedMetadataKey);
		return this.decodeData(longToUint64(metadataEntry.scopedMetadataKey), chunks);
	},

	/**
	 * Retrieve and align all metadata, may be slow if amount of data tied to it is large.
	 * @param {MetadataDb} db database
	 * @param {Uint8Array} compositeHashes metal Id
	 * @returns {Buffer} Decoded binary data.
	 */
	/**
	 * Retrieve and align all metadata, may be slow if amount of data tied to it is large.
	 * @param {MetadataDb} db database
	 * @param {Uint8Array} compositeHashes metal Id
	 * @returns {Buffer} Decoded binary data.
	 */
	async decodeDataBulk(db, compositeHashes) {
		const firstChunk = (await db.metadatasByCompositeHash(compositeHashes))[0];
		if (!firstChunk)
			throw Error('could not get first chunk, it may mistake the metal ID.');
		const { metadataEntry } = firstChunk;
		const chunks = [];
		let counter = 1;
		const fetchMetadata = async () => {
			const options = {
				sortField: 'id', sortDirection: 1, pageSize: 2000, pageNumber: counter
			};
			const c = await db.metadata(
				new Uint8Array(metadataEntry.sourceAddress.buffer),
				new Uint8Array(metadataEntry.targetAddress.buffer),
				undefined,
				metadataEntry.targetId,
				metadataEntry.metadataType,
				options
			);
			c.data.forEach(e => {
				chunks.push({
					key: longToUint64(e.metadataEntry.scopedMetadataKey),
					value: e.metadataEntry.value.buffer
				});
			});
			counter++;
			if (0 < c.data.length)
				await fetchMetadata();
		};
		await fetchMetadata();
		return this.decodeData(longToUint64(metadataEntry.scopedMetadataKey), chunks);
	}
};

module.exports = metal;
