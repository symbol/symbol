const bitmask = bitsNumber => -1 >>> (32 - bitsNumber);

const check = (byteSize, value, isSigned) => {
	let lowerBound;
	let upperBound;
	if (8 === byteSize) {
		if ('bigint' !== typeof value)
			throw new TypeError(`"value" (${value}) has invalid type, expected BigInt`);

		lowerBound = isSigned ? -0x80000000_00000000n : 0n;
		upperBound = isSigned ? 0x7FFFFFFF_FFFFFFFFn : 0xFFFFFFFF_FFFFFFFFn;
	} else {
		if (!Number.isInteger(value))
			throw new RangeError(`"value" (${value}) is not an integer`);

		// note: all expressions are tricky, they're used due to JS dumbness re 1<<32 == 0 and 1<<31 = min signed int
		lowerBound = isSigned ? -bitmask((8 * byteSize) - 1) - 1 : 0;
		upperBound = isSigned ? bitmask((8 * byteSize) - 1) : bitmask(8 * byteSize);
	}

	if (value < lowerBound || value > upperBound)
		throw RangeError(`"value" (${value}) is outside of valid ${8 * byteSize}-bit range`);

	return value;
};

const isNonNegative = value => {
	if ('bigint' === typeof value)
		return 0n <= value;

	return 0 <= value;
};

/**
 * Represents a base integer.
 */
export default class BaseValue {
	/**
	 * Creates a base value.
	 * @param {number} size Size of the integer.
	 * @param {number|bigint} value Value.
	 * @param {boolean} isSigned \c true if the value should be treated as signed.
	 */
	constructor(size, value, isSigned = false) {
		/**
		 * Size of the integer.
		 * @type number
		 */
		this.size = size;

		/**
		 * \c true if the value should be treated as signed.
		 * @type boolean
		 */
		this.isSigned = isSigned;

		/**
		 * Value.
		 * @type number|bigint
		 */
		this.value = check(size, value, isSigned);
	}

	/**
	 * Converts base value to string.
	 * @returns {string} String representation.
	 */
	toString() {
		let unsignedValue;
		if (!this.isSigned || isNonNegative(this.value)) {
			unsignedValue = this.value;
		} else {
			const upperBoundPlusOne = (8 === this.size ? 0x1_00000000_00000000n : BigInt(bitmask(this.size * 8) + 1));
			unsignedValue = BigInt(this.value) + upperBoundPlusOne;
		}

		return `0x${unsignedValue.toString(16).toUpperCase().padStart(this.size * 2, '0')}`;
	}
}
