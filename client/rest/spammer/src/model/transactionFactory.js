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

const networkTime = require('../utils/networkTime');
const random = require('../utils/random');
const catapult = require('catapult-sdk');

const { uint64 } = catapult.utils;

const createTransaction = (options, type) => ({
	verifiableEntityHeader_Reserved1: 0,
	signature: new Uint8Array(catapult.constants.sizes.signature),
	signerPublicKey: options.signerPublicKey,
	entityBody_Reserved1: 0,
	version: options.networkId + 3,
	network: options.networkId,
	type,
	transactionsHash: new Uint8Array(catapult.constants.sizes.hash256),
	aggregateTransactionHeader_Reserved1: 0,
	maxFee: uint64.fromUint(0),
	deadline: uint64.fromUint(networkTime.getNetworkTime() + (60 * 60 * 1000))
});

module.exports = {
	createRandomTransfer: (options, recipientSelector) => Object.assign(createTransaction(options, catapult.model.EntityType.transfer), {
		recipientAddress: recipientSelector(),
		message: Buffer.from(uint64.toHex(uint64.fromUint(options.transferId)), 'hex'),
		mosaics: [
			{ id: [0xD95FCF29, 0xD525AD41],	amount: uint64.fromUint(random.uint32(1000000)) }
		]
	}),

	createAggregateTransaction: (options, transactions) => Object.assign(
		createTransaction(options, catapult.model.EntityType.aggregateComplete),
		{ transactions }
	)
};
