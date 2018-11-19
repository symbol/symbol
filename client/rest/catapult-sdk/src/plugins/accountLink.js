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

/** @module plugins/accountLink */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');

const constants = { sizes };

/**
 * Creates an accountLink plugin.
 * @type {module:plugins/CatapultPlugin}
 */
const accountLinkPlugin = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.accountLink, {
			remoteAccountKey: ModelType.binary
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.accountLink, {
			deserialize: parser => {
				const transaction = {};
				transaction.remoteAccountKey = parser.buffer(constants.sizes.signer);
				transaction.linkAction = parser.uint8();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeBuffer(transaction.remoteAccountKey);
				serializer.writeUint8(transaction.linkAction);
			}
		});
	}
};

module.exports = accountLinkPlugin;
