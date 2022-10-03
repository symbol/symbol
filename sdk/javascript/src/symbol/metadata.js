/**
 * Creates a metadata payload for updating old value to new value.
 * @param {Uint8Array} oldValue Old metadata value.
 * @param {Uint8Array} newValue New metadata value.
 * @returns {Uint8Array} Metadata payload for updating old value to new value.
 */
export const metadataUpdateValue = (oldValue, newValue) => { // eslint-disable-line import/prefer-default-export
	if (!oldValue)
		return newValue;

	const shorterLength = Math.min(oldValue.length, newValue.length);
	const longerLength = Math.max(oldValue.length, newValue.length);
	const isNewValueShorter = oldValue.length > newValue.length;

	const result = new Uint8Array(longerLength);

	let i = 0;
	for (i = 0; i < shorterLength; ++i)
		result[i] = oldValue[i] ^ newValue[i];

	for (; i < longerLength; ++i)
		result[i] = (isNewValueShorter ? oldValue : newValue)[i];

	return result;
};
