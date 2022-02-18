const { BufferView } = require('./BufferView');

const readArrayImpl = (bufferInput, FactoryClass, accessor, shouldContinue) => {
	const view = new BufferView(bufferInput);
	const elements = [];
	let previousElement = null;
	let i = 0;
	while (shouldContinue(i, view)) {
		const element = FactoryClass.deserialize(view.buffer);

		if (0 >= element.size)
			throw RangeError('element size has invalid size');

		if (accessor && previousElement && accessor(previousElement) >= accessor(element))
			throw RangeError('elements in array are not sorted');

		elements.push(element);
		view.shiftRight(element.size);

		previousElement = element;
		++i;
	}

	return elements;
};

const writeArrayImpl = (output, elements, count, accessor = null) => {
	for (let i = 0; i < count; ++i) {
		const element = elements[i];
		if (accessor && 0 < i && accessor(elements[i - 1]) >= accessor(element))
			throw RangeError('array passed to write array is not sorted');

		output.write(element.serialize());
	}
};

const sum = numbers => numbers.reduce((a, b) => a + b, 0);

const arrayHelpers = {
	/**
	 * Calculates aligned size.
	 * @param {number} size Size.
	 * @param {number} alignment Alignment.
	 * @returns {number} Size rounded up to alignment.
	 */
	alignUp: (size, alignment) => Math.floor((size + alignment - 1) / alignment) * alignment,

	/**
	 * Calculates size of variable size objects.
	 * @param {array<object>} elements Serializable elements.
	 * @param {number} alignment Alignment used for calculations.
	 * @param {boolean} skipLastElementPadding true if last element should not be aligned.
	 * @returns {number} Computed size.
	 */
	size: (elements, alignment = 0, skipLastElementPadding = undefined) => {
		if (!alignment)
			return sum(elements.map(e => e.size));

		if (!skipLastElementPadding)
			return sum(elements.map(e => arrayHelpers.alignUp(e.size, alignment)));

		return sum(elements.slice(0, -1).map(e => arrayHelpers.alignUp(e.size, alignment))) + sum(elements.slice(-1).map(e => e.size));
	},

	/**
	 * Reads array of objects.
	 * @param {Uint8Array} bufferInput Input buffer.
	 * @param {type} FactoryClass Factory used to deserialize objects.
	 * @param {function} accessor Optional accessor used to check objects order.
	 * @returns {array<object>} Array of deserialized objects.
	 */
	readArray: (bufferInput, FactoryClass, accessor = null) =>
		// note: this method is used only for '__FILL__' type arrays
		// this loop assumes properly sliced buffer is passed and that there's no additional data.
		readArrayImpl(bufferInput, FactoryClass, accessor, (_, view) => 0 < view.buffer.length),

	/**
	 * Reads array of deterministic number of objects.
	 * @param {Uint8Array} bufferInput A uint8 array.
	 * @param {type} FactoryClass Factory used to deserialize objects.
	 * @param {number} count Number of object to deserialize.
	 * @param {function} accessor Optional accessor used to check objects order.
	 * @returns {array<object>} Array of deserialized objects.
	 */
	readArrayCount: (bufferInput, FactoryClass, count, accessor = null) =>
		readArrayImpl(bufferInput, FactoryClass, accessor, index => count > index),

	/**
	 * Reads array of variable size objects.
	 * @param {Uint8Array} bufferInput A uint8 array.
	 * @param {type} FactoryClass Factory used to deserialize objects.
	 * @param {number} alignment Alignment used to make sure each object is at boundary.
	 * @param {boolean} skipLastElementPadding true if last element is not aligned/padded.
	 * @returns {array<object>} Array of deserialized objects.
	 */
	readVariableSizeElements: (bufferInput, FactoryClass, alignment, skipLastElementPadding = false) => {
		const view = new BufferView(bufferInput);
		const elements = [];
		while (0 < view.buffer.length) {
			const element = FactoryClass.deserialize(view.buffer);

			if (0 >= element.size)
				throw RangeError('element size has invalid size');

			elements.push(element);

			const alignedSize = (skipLastElementPadding && element.size >= view.buffer.length)
				? element.size
				: arrayHelpers.alignUp(element.size, alignment);
			if (alignedSize > view.buffer.length)
				throw RangeError('unexpected buffer length');

			view.shiftRight(alignedSize);
		}

		return elements;
	},

	/**
	 * Writes array of objects.
	 * @param {Writer} output An output sink.
	 * @param {array<object>} elements Serializable elements.
	 * @param {function} accessor Optional accessor used to check objects order.
	 */
	writeArray: (output, elements, accessor = undefined) => {
		writeArrayImpl(output, elements, elements.length, accessor);
	},

	/**
	 * Writes array of deterministic number of objects.
	 * @param {Writer} output An output sink.
	 * @param {array<object>} elements Serializable elements.
	 * @param {number} count Number of objects to write.
	 * @param {function} accessor Optional accessor used to check objects order.
	 */
	writeArrayCount: writeArrayImpl,

	/**
	 * Writes array of variable size objects.
	 * @param {Writer} output An output sink.
	 * @param {array<object>} elements Serializable elements.
	 * @param {number} alignment Alignment used to make sure each object is at boundary.
	 * @param {boolean} skipLastElementPadding true if last element should not be aligned/padded.
	 */
	writeVariableSizeElements: (output, elements, alignment, skipLastElementPadding = false) => {
		elements.forEach((element, index) => {
			output.write(element.serialize());
			if (!skipLastElementPadding || elements.length - 1 !== index) {
				const alignedSize = arrayHelpers.alignUp(element.size, alignment);
				if (alignedSize - element.size)
					output.write(new Uint8Array(alignedSize - element.size));
			}
		});
	}
};

module.exports = arrayHelpers;
