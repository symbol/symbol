/**
 * Buffer view.
 */
export default class BufferView {
	/**
	 * Creates buffer view around a buffer.
	 * @param {Uint8Array} buffer Initial buffer view.
	 */
	constructor(buffer) {
		/**
		 * Underlying buffer view.
		 * @type Uint8Array
		 */
		this.buffer = buffer;
	}

	/**
	 * Moves view right.
	 * @param {number} size Amount of bytes to shift.
	 */
	shiftRight(size) {
		this.buffer = new Uint8Array(this.buffer.buffer, this.buffer.byteOffset + size, this.buffer.length - size);
	}

	/**
	 * Returns new limited view.
	 * @param {number} size Length in bytes.
	 * @returns {Uint8Array} View limited to specified size.
	 */
	window(size) {
		if (size > this.buffer.length)
			throw RangeError(`invalid shrink value: ${size} vs ${this.buffer.length}`);

		return new Uint8Array(this.buffer.buffer, this.buffer.byteOffset, size);
	}

	/**
	 * Shrinks view to specified size
	 * @param {number} size New length in bytes.
	 */
	shrink(size) {
		this.buffer = this.window(size);
	}
}
