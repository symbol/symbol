/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const catapult = require('catapult-sdk');

const { BinaryParser } = catapult.parser;
const { uint64 } = catapult.utils;

const parserFromData = binaryData => {
	const parser = new catapult.parser.BinaryParser();
	parser.push(binaryData);
	return parser;
};

const createBlockDescriptor = () => ({
	filter: topicParam => {
		if (topicParam)
			throw new Error('unexpected param to block subscription');

		return Buffer.of(0x49, 0x6A, 0xCA, 0x80, 0xE4, 0xD8, 0xF2, 0x9F);
	},

	handler: (codec, emit) => (topic, binaryBlock, hash, generationHash) => {
		const block = codec.deserialize(parserFromData(binaryBlock), { skipBlockTransactions: true });
		emit({ type: 'blockHeaderWithMetadata', payload: { block, meta: { hash, generationHash } } });
	}
});

const createPolicyBasedAddressFilter = (markerByte, emptyAddressHandler) => topicParam => {
	if (!topicParam)
		return emptyAddressHandler(markerByte);

	return Buffer.concat([Buffer.of(markerByte), Buffer.from(catapult.model.address.stringToAddress(topicParam))]);
};

const handlers = {
	transaction: channelName => (codec, emit) => (topic, binaryTransaction, hash, merkleComponentHash, height) => {
		const transaction = codec.deserialize(parserFromData(binaryTransaction));
		const meta = {
			hash, merkleComponentHash, height: uint64.fromBytes(height), channelName
		};
		emit({ type: 'transactionWithMetadata', payload: { transaction, meta } });
	},

	transactionHash: channelName => (codec, emit) => (topic, hash) => {
		emit({ type: 'transactionWithMetadata', payload: { meta: { hash, channelName } } });
	}
};

/**
 * Builder for creating message channel information.
 */
class MessageChannelBuilder {
	/**
	 * Creates a builder.
	 * @param {object} config Message queue configuration.
	 */
	constructor(config) {
		this.descriptors = {};
		this.channelMarkers = {};

		const emptyAddressHandler = config && config.allowOptionalAddress
			? markerByte => Buffer.of(markerByte)
			: () => { throw new Error('address param missing from address subscription'); };
		this.createAddressFilter = markerChar => createPolicyBasedAddressFilter(markerChar.charCodeAt(0), emptyAddressHandler);

		// add basic descriptors
		this.descriptors.block = createBlockDescriptor();
		this.add('confirmedAdded', 'a', 'transaction');
		this.add('unconfirmedAdded', 'u', 'transaction');
		this.add('unconfirmedRemoved', 'r', 'transactionHash');
		this.descriptors.status = {
			filter: this.createAddressFilter('s'),
			handler: (codec, emit) => (topic, buffer) => {
				const parser = new BinaryParser();
				parser.push(buffer);

				const hash = parser.buffer(catapult.constants.sizes.hash256);
				const status = parser.uint32();
				const deadline = parser.uint64();
				emit({ type: 'transactionStatus', payload: { hash, status, deadline } });
			}
		};
	}

	/**
	 * Adds support for a new channel.
	 * @param {string} name The channel name.
	 * @param {string} markerChar The channel marker character.
	 * @param {function} handler The channel data handler.
	 */
	add(name, markerChar, handler) {
		if (name in this.descriptors)
			throw Error(`'${name}' channel has already been registered`);

		if (1 !== markerChar.length)
			throw Error('channel marker must be single character');

		if (markerChar in this.channelMarkers)
			throw Error(`'${markerChar}' channel marker has already been registered`);

		let channelHandler = handler;
		if ('string' === typeof handler) {
			if (!(handler in handlers))
				throw Error(`cannot register channel '${name}' with unknown handler '${handler}'`);

			channelHandler = handlers[handler](name);
		}

		this.descriptors[name] = { filter: this.createAddressFilter(markerChar), handler: channelHandler };
		this.channelMarkers[markerChar] = 1;
	}

	/**
	 * Builds and returns an object composed of all configured channel information.
	 * @returns {object} An object composed of all configured channel information.
	 */
	build() {
		return this.descriptors;
	}
}

module.exports = MessageChannelBuilder;
