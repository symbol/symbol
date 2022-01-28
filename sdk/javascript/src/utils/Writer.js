class Writer {
	constructor(size) {
		this.storage = new Uint8Array(size);
		this.offset = 0;
	}

	write(arrayInput) {
		this.storage.set(arrayInput, this.offset);
		this.offset += arrayInput.length;
	}
}

module.exports = { Writer };
