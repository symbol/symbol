function random(max) {
	if (0 > max || 0xFFFFFFFF < max)
		throw Error(`${max} does not fit into 32 bits`);

	return Math.floor(Math.random() * max);
}

function toUint64(value) {
	if (value > Number.MAX_SAFE_INTEGER)
		throw new Error(`${value} does not fit into 53 bits`);

	return [value >>> 0, Math.floor(value / 4294967296)];
}

function uint32ToBytes(value) {
	if (0 > value || 0xFFFFFFFF < value)
		throw Error(`${value} does not fit into 32 bits`);

	return Uint8Array.of([
		(value & 0x000000FF),
		(value & 0x0000FF00) >>> 8,
		(value & 0x00FF0000) >>> 16,
		(value & 0xFF000000) >>> 24
	]);
}

export default { random, toUint64, uint32ToBytes };
