/*
 * Copyright (c) 2016-2019, Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp.
 * Copyright (c) 2020-present, Jaguar0625, gimre, BloodyRookie.
 * All rights reserved.
 *
 * This file is part of Catapult.
 *
 * Catapult is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Catapult is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Catapult.  If not, see <http://www.gnu.org/licenses/>.
 */

/** @module parser/PacketParser */
const BinaryParser = require('./BinaryParser');
const EventEmitter = require('events');

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
 * @property {numeric} type Packet type.
 * @property {numeric} size Packet size.
 * @property {Buffer} payload Packet payload.
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
	 * @param {Buffer} buffer Binary buffer.
	 */
	push(buffer) {
		this.impl.push(buffer);
	}

	/**
	 * Subscribes a handler to receive notifications when full packets have been received.
	 * @param {Function} handler Handler function that is called with a {@link module:parser/PacketParser~RawPacket}.
	 */
	onPacket(handler) {
		this.impl.emitter.on('packet', handler);
	}
}

module.exports = PacketParser;
