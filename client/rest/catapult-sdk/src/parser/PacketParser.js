/** @module parser/PacketParser */
const EventEmitter = require('events');
const BinaryParser = require('./BinaryParser');

const Packet_Header_Size = 8;

class PacketParserImpl {
	constructor() {
		this.parser = new BinaryParser();
		this.packetHeader = undefined;
		this.emitter = new EventEmitter();
	}

	push(buffer) {
		this.parser.push(buffer);

		// note: processHeader will process at most one header when called consecutively
		do {
			this.processHeader();
			this.processBody();
		} while (this.processHeader());
	}

	processHeader() {
		if (undefined !== this.packetHeader || this.parser.numUnprocessedBytes() < Packet_Header_Size)
			return false;

		const size = this.parser.uint32();
		const type = this.parser.uint32();

		if (size < Packet_Header_Size)
			throw Error(`packet size (${size}) cannot be less than packet header size`);

		this.packetHeader = { size, type };
		return true;
	}

	processBody() {
		if (undefined === this.packetHeader || this.parser.numUnprocessedBytes() < this.packetHeader.size - Packet_Header_Size)
			return false;

		this.raisePacketEvent();
		this.packetHeader = undefined;
		return true;
	}

	raisePacketEvent() {
		// consume the entire payload
		const payload = this.packetHeader.size > Packet_Header_Size
			? this.parser.buffer(this.packetHeader.size - Packet_Header_Size)
			: Buffer.alloc(0);

		this.emitter.emit('packet', { type: this.packetHeader.type, size: this.packetHeader.size, payload });
	}
}

/**
 * A raw packet composed of header information and a payload.
 * @typedef {object} RawPacket
 * @property {numeric} type The packet type.
 * @property {numeric} size The packet size.
 * @property {Buffer} payload The packet payload.
 */

/**
 * Accepts and buffers binary data and emits events when full packets have been received.
 */
class PacketParser {
	/**
	 * Creates a packet parser.
	 */
	constructor() {
		this.impl = new PacketParserImpl();
	}

	/**
	 * Accepts a binary buffer and appends it to the end of the working buffer.
	 * @param {Buffer} buffer The binary buffer.
	 */
	push(buffer) {
		this.impl.push(buffer);
	}

	/**
	 * Subscribes a handler to receive notifications when full packets have been received.
	 * @param {Function} handler The handler function that is called with a {@link module:parser/PacketParser~RawPacket}.
	 */
	onPacket(handler) {
		this.impl.emitter.on('packet', handler);
	}
}

module.exports = PacketParser;
