const catapult = require('../../catapult-sdk');
const bs58 = require('bs58');
const { longToUint64 } = require('../../db/dbUtils');
const  { sha3_256 } = require('@noble/hashes/sha3');
const MetadataDb = require('./MetadataDb.js');
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
		return { compositeHash: hashHex.slice(METAL_ID_HEADER_HEX.length)};
	},

  /**
   * Split and return the value of a metadata entry according to the metadata protocol.
   * @param {Buffer} b 
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
	 * @param {Buffer} input 
	 * @returns {Uint32Array} [Uint32, Uint32]
	 */
	generateMetadataKey(input) {
		if (input.length === 0) {
			throw new Error("Input must not be empty");
		}
		const buf = sha3_256(input).buffer;
    const result = new Uint32Array(buf);
    return [result[0], result[1] & 0x7FFFFFFF];
	},

	/**
	 * Decode binary data from chunks.
	 * @param {Uint64} fitstKey
	 * @param {Array} chunks
	 * @returns {Buffer} Decoded binary data.
	 */
	decodeData (fitstKey, chunks) {
		let scopedMetadataKey = fitstKey;
		let valueResult = Buffer.from([]);
		do {
			const chunk = chunks.find(chunk => catapult.utils.uint64.compare(chunk.key, scopedMetadataKey) == 0);
			if(!chunk) throw new Error(`Error: The chunk ${scopedMetadataKey} is missing`);
			const checksum = this.generateMetadataKey(chunk.value);
			const uintScopedMetadataKey = scopedMetadataKey;
			if(checksum[0] != uintScopedMetadataKey[0] || checksum[1] != uintScopedMetadataKey[1]) {
				throw new Error(`Error: The chunk ${scopedMetadataKey} is broken (calculated=${checksum})`);
			}
			const c = this.splitBuffer(chunk.value);
			scopedMetadataKey = c.scopedMetadataKey;
			magic = c.magic;
			valueResult = Buffer.concat([valueResult, c.value]);
		} while (0x80 !== (magic[0] & 0x80));
		return valueResult;
	},

	/**
	 * Read and decode ScopedMetadaKey in order, usually not used because it is slow.
	 * @param {MetadataDb} db database
	 * @param {Uint8Array} compositeHashes metal Id
	 * @returns {Buffer} Decoded binary data.
	 */
	async decodeDataStepByStep (db, compositeHashes) {
		const options = {
			sortField: 'id', sortDirection: 1, pageSize: 1, pageNumber: 1
		};
		let chunks = [];
		const firstChunk = (await db.metadatasByCompositeHash(compositeHashes))[0];
		if (!firstChunk) throw Error('could not get first chunk, it may mistake the metal ID.');
		chunks.push({
			key: longToUint64(firstChunk.metadataEntry.scopedMetadataKey),
			value: firstChunk.metadataEntry.value.buffer
		});
		const metadataEntry = firstChunk.metadataEntry;
		const metadata = this.splitBuffer(metadataEntry.value.buffer);
		let magic = metadata.magic;
		let scopedMetadataKey = metadata.scopedMetadataKey;
		do {
			let c = await db.metadata(
				new Uint8Array(metadataEntry.sourceAddress.buffer),
				new Uint8Array(metadataEntry.targetAddress.buffer),
				scopedMetadataKey,
				metadataEntry.targetId,
				metadataEntry.metadataType,
				options
			).then(r => r.data);
			if (c.length == 0) break;
			chunks.push({
				key: longToUint64(c[0].metadataEntry.scopedMetadataKey),
				value: c[0].metadataEntry.value.buffer
			});
			const m = this.splitBuffer(c[0].metadataEntry.value.buffer);
			magic = m.magic;
			scopedMetadataKey = m.scopedMetadataKey;
		} while (0x80 !== (magic[0] & 0x80));
		return this.decodeData(longToUint64(metadataEntry.scopedMetadataKey), chunks);
	},

	/**
	 * Retrieve and align all metadata, may be slow if amount of data tied to it is large.
	 * @param {MetadataDb} db database
	 * @param {Uint8Array} compositeHashes metal Id
	 * @returns {Buffer} Decoded binary data.
	 */
	async decodeDataBulk(db, compositeHashes) {
		const firstChunk = (await db.metadatasByCompositeHash(compositeHashes))[0];
		if (!firstChunk) throw Error('could not get first chunk, it may mistake the metal ID.');
		const metadataEntry = firstChunk.metadataEntry;
		let chunks = [];
		let counter = 1;
		do {
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
			).then(r => r.data);
			if (c.length == 0) break;
			c.forEach(e => {
				chunks.push({
					key: longToUint64(e.metadataEntry.scopedMetadataKey),
					value: e.metadataEntry.value.buffer
				});
			});
			counter++;
		} while (true);
		return this.decodeData(longToUint64(metadataEntry.scopedMetadataKey), chunks);
	}
};

module.exports = metal;