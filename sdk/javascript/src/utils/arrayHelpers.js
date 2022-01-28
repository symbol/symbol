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
			throw RangeError('array passed to writeArray is not sorted');

		output.write(element.serialize());
	}
};

const arrayHelpers = {
	/**
	 * Calculates aligned size.
	 * @param {number} size Size.
	 * @param {number} alignment Alignment.
	 * @returns {number} Size rounded up to alignment.
	 */
	alignUp: (size, alignment) => Math.floor((size + alignment - 1) / alignment) * alignment,

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
	 * Reads array of objects.
	 * @param {Uint8Array} bufferInput A uint8 array.
	 * @param {type} FactoryClass Factory used to deserialize objects.
	 * @param {number} count Number of object to deserialize.
	 * @param {function} accessor Optional accessor used to check objects order.
	 * @returns {array<object>} Array of deserialized objects.
	 */
	readArrayCount: (bufferInput, FactoryClass, count, accessor = null) =>
		readArrayImpl(bufferInput, FactoryClass, accessor, index => count > index),

	/**
	 * Reads array of objects.
	 * @param {Uint8Array} bufferInput A uint8 array.
	 * @param {type} FactoryClass Factory used to deserialize objects.
	 * @param {number} alignment Alignment used to make sure each object is at boundary.
	 * @returns {array<object>} Array of deserialized objects.
	 */
	readVariableSizeElements: (bufferInput, FactoryClass, alignment) => {
		const view = new BufferView(bufferInput);
		const elements = [];
		while (0 < view.buffer.length) {
			const element = FactoryClass.deserialize(view.buffer);

			if (0 >= element.size)
				throw RangeError('element size has invalid size');

			elements.push(element);

			const alignedSize = arrayHelpers.alignUp(element.size, alignment);
			if (alignedSize > view.buffer.length)
				throw RangeError('unexpected buffer length');

			view.shiftRight(alignedSize);
		}

		return elements;
	},

	/**
	 * Write out objects.
	 * @param {Writer} output An output sink.
	 * @param {array<object>} elements Serializable elements.
	 * @param {function} accessor Optional accessor used to check objects order.
	 */
	writeArray: (output, elements, accessor = undefined) => {
		writeArrayImpl(output, elements, elements.length, accessor);
	},

	/**
	 * Write out objects.
	 * @param {Writer} output An output sink.
	 * @param {array<object>} elements Serializable elements.
	 * @param {number} count Number of objects to write.
	 * @param {function} accessor Optional accessor used to check objects order.
	 */
	writeArrayCount: writeArrayImpl,

	/**
	 * Write out objects.
	 * @param {Writer} output An output sink.
	 * @param {array<object>} elements Serializable elements.
	 * @param {number} alignment Alignment used to make sure each object is at boundary.
	 */
	writeVariableSizeElements: (output, elements, alignment) => {
		elements.forEach(element => {
			output.write(element.serialize());
			const alignedSize = arrayHelpers.alignUp(element.size, alignment);
			if (alignedSize - element.size)
				output.write(new Uint8Array(alignedSize - element.size));
		});
	}
};

module.exports = arrayHelpers;
