/** @module modelBinary/blockHeaderCodec */
import sizes from './sizes';

const constants = { sizes };

export default {
	/**
	 * Parses a block header.
	 * @param {object} parser The parser.
	 * @returns {object} The parsed block header.
	 */
	deserialize: parser => {
		const blockHeader = {};
		blockHeader.height = parser.uint64();
		blockHeader.timestamp = parser.uint64();
		blockHeader.difficulty = parser.uint64();
		blockHeader.previousBlockHash = parser.buffer(constants.sizes.hash);
		blockHeader.blockTransactionsHash = parser.buffer(constants.sizes.hash);
		return blockHeader;
	},

	/**
	 * Serializes a block header.
	 * @param {object} blockHeader The block header.
	 * @param {object} serializer The serializer.
	 */
	serialize: (blockHeader, serializer) => {
		serializer.writeUint64(blockHeader.height);
		serializer.writeUint64(blockHeader.timestamp);
		serializer.writeUint64(blockHeader.difficulty);
		serializer.writeBuffer(blockHeader.previousBlockHash);
		serializer.writeBuffer(blockHeader.blockTransactionsHash);
	}
};
