const bitmask = bitsNumber => -1 >>> (32 - bitsNumber);

const check = (byteSize, value, isSigned) => {
	let lowerBound;
	let upperBound;
	if (8 === byteSize) {
		if ('bigint' !== typeof value)
			throw new TypeError(`"value" (${value}) has invalid type`);

		lowerBound = isSigned ? -BigInt('0x8000000000000000') : BigInt(0);
		upperBound = isSigned ? BigInt('0x7FFFFFFFFFFFFFFF') : BigInt('0xFFFFFFFFFFFFFFFF');
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

/**
 * Represents a base integer.
 */
class BaseValue {
	/**
	 * Creates a base value.
	 * @param {number} size Size of the integer.
	 * @param {number|BigInt} value Value.
	 * @param {boolean} isSigned Should the value be treated as signed.
	 */
	constructor(size, value, isSigned = false) {
		this.size = size;
		this.isSigned = isSigned;
		this.value = check(size, value, isSigned);
	}
}

module.exports = { BaseValue };
