module.exports = {
	uint32: max => {
		if (0 > max || 0xFFFFFFFF < max)
			throw Error(`${max} does not fit into 32 bits`);

		return Math.floor(Math.random() * max);
	}
};
