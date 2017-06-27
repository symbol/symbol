/** @exports packet/header */
const packetHeader = {
	/**
	 * @property {numeric} size The size (in bytes) of a packet header.
	 */
	size: 8,

	/**
	 * Creates a packet header buffer.
	 * @param {module:packet/types} type The packet type.
	 * @param {numeric} size The packet size.
	 * @returns {Buffer} The packet header buffer.
	 */
	createBuffer: (type, size) => {
		const header = Buffer.alloc(packetHeader.size);
		header.writeInt32LE(size, 0);
		header.writeInt32LE(type, 4);
		return header;
	}
};

export default packetHeader;
