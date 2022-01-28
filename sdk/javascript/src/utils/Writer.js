/**
 * Position tracking writer.
 */
class Writer {
	/**
	 * Creates a writer with specified size.
	 * @param {number} size Allocated buffer size.
	 */
	constructor(size) {
		this.storage = new Uint8Array(size);
		this.offset = 0;
	}

	/**
	 * Writes array into buffer.
	 * @param {array<byte>} arrayInput Data to write.
	 */
	write(arrayInput) {
		this.storage.set(arrayInput, this.offset);
		this.offset += arrayInput.length;
	}
}

module.exports = { Writer };
