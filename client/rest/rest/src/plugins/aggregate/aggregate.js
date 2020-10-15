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

/** @module plugins/aggregate */
const aggregateRoutes = require('./aggregateRoutes');
const { ServerMessageHandler } = require('../../connection/serverMessageHandlers');
const catapult = require('catapult-sdk');

const { BinaryParser } = catapult.parser;

/**
 * Creates an aggregate plugin.
 * @type {module:plugins/CatapultRestPlugin}
 */
module.exports = {
	createDb: () => {},

	registerTransactionStates: states => {
		states.push({ friendlyName: 'partial', dbPostfix: 'Partial', routePostfix: '/partial' });
	},

	registerMessageChannels: builder => {
		builder.add('partialAdded', 'p', ServerMessageHandler.transaction);
		builder.add('partialRemoved', 'q', ServerMessageHandler.transactionHash);
		builder.add('cosignature', 'c', (codec, emit) => (topic, buffer) => {
			const parser = new BinaryParser();
			parser.push(buffer);

			const version = parser.uint64();
			const signerPublicKey = parser.buffer(catapult.constants.sizes.signerPublicKey);
			const signature = parser.buffer(catapult.constants.sizes.signature);
			const parentHash = parser.buffer(catapult.constants.sizes.hash256);
			emit({
				type: 'aggregate.cosignature',
				payload: {
					version,
					signerPublicKey,
					signature,
					parentHash
				}
			});
		});
	},

	registerRoutes: (...args) => {
		aggregateRoutes.register(...args);
	}
};
