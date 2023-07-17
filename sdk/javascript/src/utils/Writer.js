/**
 * Position tracking writer.
 */
export default class Writer {
	/**
	 * Creates a writer with specified size.
	 * @param {number} size Allocated buffer size.
	 */
	constructor(size) {
		/**
		 *  Underlying storage.
		 * @type Uint8Array
		 */
		this.storage = new Uint8Array(size);

		/**
		 * Current offset.
		 * @type number
		 */
		this.offset = 0;
	}

	/**
	 * Writes array into buffer.
	 * @param {Uint8Array|Array<number>} buffer Data to write.
	 */
	write(buffer) {
		this.storage.set(buffer, this.offset);
		this.offset += buffer.length;
	}
}
