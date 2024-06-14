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

import ServerMessageHandler from './serverMessageHandlers.js';
import { utils } from 'symbol-sdk';
import { Address, models } from 'symbol-sdk/symbol';

const createBlockDescriptor = (marker, handler) => ({
	filter: topicParam => {
		if (topicParam)
			throw new Error('unexpected param to block subscription');

		return marker;
	},
	handler
});

const createPolicyBasedAddressFilter = (markerByte, emptyAddressHandler, networkIdentifier) => topicParam => {
	if (!topicParam)
		return emptyAddressHandler(markerByte);

	// If the sent param is an namespace id hex like C0FB8AA409916260
	if (utils.isHexString(topicParam) && 16 === topicParam.length) {
		const namespaceId = new models.NamespaceId(BigInt(`0x${topicParam}`));
		const address = Address.fromNamespaceId(namespaceId, networkIdentifier);
		return Buffer.concat([Buffer.of(markerByte), Buffer.from(address.bytes)]);
	}
	// When it's a encoded address, e.g. TAHNZXQBC57AA7KJTMGS3PJPZBXN7DV5JHJU42A
	try {
		const addressByteArray = new Address(topicParam).bytes;
		return Buffer.concat([Buffer.of(markerByte), Buffer.from(addressByteArray)]);
	} catch (err) {
		throw Error(`${topicParam} does not represent a valid encoded address`);
	}
};

/**
 * Builder for creating message channel information.
 */
export default class MessageChannelBuilder {
	/**
	 * Creates a builder.
	 * @param {object} config Message queue configuration.
	 * @param {number} networkIdentifier the network identifier
	 */
	constructor(config, networkIdentifier) {
		this.descriptors = {};
		this.channelMarkers = {};

		const emptyAddressHandler = config && config.allowOptionalAddress
			? markerByte => Buffer.of(markerByte)
			: () => { throw new Error('address param missing from address subscription'); };

		this.createAddressFilter = markerChar =>
			createPolicyBasedAddressFilter(markerChar.charCodeAt(0), emptyAddressHandler, networkIdentifier);

		// add basic descriptors
		this.descriptors.block = createBlockDescriptor(
			Buffer.of(0x49, 0x6A, 0xCA, 0x80, 0xE4, 0xD8, 0xF2, 0x9F),
			ServerMessageHandler.block
		);
		this.descriptors.finalizedBlock = createBlockDescriptor(
			Buffer.of(0x54, 0x79, 0xCE, 0x31, 0xA0, 0x32, 0x48, 0x4D),
			ServerMessageHandler.finalizedBlock
		);
		this.add('confirmedAdded', 'a', ServerMessageHandler.transaction);
		this.add('unconfirmedAdded', 'u', ServerMessageHandler.transaction);
		this.add('unconfirmedRemoved', 'r', ServerMessageHandler.transactionHash);
		this.descriptors.status = {
			filter: this.createAddressFilter('s'),
			handler: ServerMessageHandler.transactionStatus
		};
	}

	/**
	 * Adds support for a new channel.
	 * @param {string} name Channel name.
	 * @param {string} markerChar Channel marker character.
	 * @param {Function} handler Channel data handler.
	 */
	add(name, markerChar, handler) {
		if (name in this.descriptors)
			throw Error(`'${name}' channel has already been registered`);

		if (1 !== markerChar.length)
			throw Error('channel marker must be single character');

		if (markerChar in this.channelMarkers)
			throw Error(`'${markerChar}' channel marker has already been registered`);

		this.descriptors[name] = { filter: this.createAddressFilter(markerChar), handler };
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
