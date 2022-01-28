const arrayHelpers = {
	align_up: (size, alignment) => Math.floor((size + alignment - 1) / alignment) * alignment,

	read_array: (bufferInput, factoryType, accessor = null) => {
		const elements = [];
		let previousElement = null;
		let buffer = bufferInput;
		// note: this method is used only for '__FILL__' type arrays
		// this loop assumes properly sliced buffer is passed and that there's no additional data.
		while (0 < buffer.length) {
			const element = factoryType.deserialize(buffer);
			if (accessor && previousElement && accessor(previousElement) >= accessor(element))
				throw RangeError('elements in array are not sorted');

			elements.push(element);
			buffer = new Uint8Array(buffer.buffer, buffer.byteOffset + element.size);

			previousElement = element;
		}

		return elements;
	},
	read_array_count: (bufferInput, factoryType, count, accessor = null) => {
		const elements = [];
		let previousElement = null;
		let buffer = bufferInput;
		for (let i = 0; i < count; ++i) {
			const element = factoryType.deserialize(buffer);
			if (accessor && previousElement && accessor(previousElement) >= accessor(element))
				throw RangeError('elements in array are not sorted');

			elements.push(element);
			buffer = new Uint8Array(buffer.buffer, buffer.byteOffset + element.size);
			previousElement = element;
		}

		return elements;
	},
	read_variable_size_elements: (bufferInput, factoryType, alignment) => {
		const elements = [];
		let buffer = bufferInput;
		while (0 < buffer.length) {
			const element = factoryType.deserialize(buffer);
			elements.push(element);

			if (0 >= element.size)
				throw RangeError('element size has invalid size');

			const alignedSize = arrayHelpers.align_up(element.size, alignment);
			if (alignedSize > buffer.length)
				throw RangeError('unexpected buffer length');

			buffer = new Uint8Array(buffer.buffer, buffer.byteOffset + alignedSize, buffer.length - alignedSize);
		}

		return elements;
	},
	write_array: (output, elements, accessor = null) => {
		elements.forEach((element, index) => {
			if (accessor && 0 < index && accessor(elements[index - 1]) >= accessor(element))
				throw RangeError('array passed to write_array is not sorted');

			output.write(element.serialize());
		});
	},
	write_array_count: () => {
		throw Error('write_array_count');
	},
	write_variable_size_elements: (output, elements, alignment) => {
		elements.forEach(element => {
			output.write(element.serialize());
			const alignedSize = arrayHelpers.align_up(element.size, alignment);
			output.write(new Uint8Array(alignedSize - element.size));
		});
	}
};

module.exports = arrayHelpers;
