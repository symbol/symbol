/**
 * Buffer view.
 */
class BufferView {
	constructor(buffer) {
		this.byteArray = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.length);
	}

	shift(size) {
		this.byteArray = new Uint8Array(this.byteArray.buffer, this.byteArray.byteOffset + size, this.byteArray.length - size);
	}

	get buffer() {
		return this.byteArray;
	}

	shrinked_buffer(size) {
		if (size > this.byteArray.length)
			throw RangeError(`Invalid shrink value: ${size} vs current size: ${this.byteArray.length}`);

		return new Uint8Array(this.byteArray.buffer, this.byteArray.byteOffset, size);
	}

	shrink(size) {
		this.byteArray = this.shrinked_buffer(size);
	}
}

module.exports = { BufferView };
