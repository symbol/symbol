/**
 * Creates a metadata payload for updating old value to new value.
 * @param {Uint8Array} oldValue Old metadata value.
 * @param {Uint8Array} newValue New metadata value.
 * @returns {Uint8Array} Metadata payload for updating old value to new value.
 */
const metadataUpdateValue = (oldValue, newValue) => {
	if (!oldValue)
		return newValue;

	const result = new Uint8Array(newValue.length);

	let i = 0;
	for (i = 0; i < Math.min(oldValue.length, newValue.length); ++i)
		result[i] = oldValue[i] ^ newValue[i];

	for (; i < newValue.length; ++i)
		result[i] = newValue[i];

	return result;
};

module.exports = { metadataUpdateValue };
