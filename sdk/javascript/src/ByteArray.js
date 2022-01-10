const converter = require('./utils/converter');

/**
 * Represents a fixed size byte array.
 */
class ByteArray {
	/**
	 * Creates a byte array.
	 * @param {number} fixedSize Size of the array.
	 * @param {Uint8Array|string} arrayInput Byte array or hex string.
	 */
	constructor(fixedSize, arrayInput) {
		let rawBytes = arrayInput;
		if ('string' === typeof rawBytes)
			rawBytes = converter.hexToUint8(rawBytes);

		if (fixedSize !== rawBytes.length)
			throw RangeError(`bytes was size ${rawBytes.length} but must be ${fixedSize}`);

		this.bytes = new Uint8Array(rawBytes);
	}

	/**
	 * Returns string representation of this object.
	 * @returns {string} String representation of this object
	 */
	toString() {
		return converter.uint8ToHex(this.bytes);
	}
}

module.exports = { ByteArray };
