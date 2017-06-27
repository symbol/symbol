/** @module modelBinary/transactionCodec */

export default {
	/**
	 * Parses a transaction.
	 * @param {object} parser The parser.
	 * @returns {object} The parsed transaction.
	 */
	deserialize: parser => {
		const transaction = {};
		transaction.fee = parser.uint64();
		transaction.deadline = parser.uint64();
		return transaction;
	},

	/**
	 * Serializes a transaction.
	 * @param {object} transaction The transaction.
	 * @param {object} serializer The serializer.
	 */
	serialize: (transaction, serializer) => {
		serializer.writeUint64(transaction.fee);
		serializer.writeUint64(transaction.deadline);
	}
};
