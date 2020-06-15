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

const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');

const { restriction } = catapult.model;
const { uint64 } = catapult.utils;

module.exports = {
	register: (server, db) => {
		const accountRestrictionsSender = routeUtils.createSender('accountRestrictions');
		const mosaicGlobalRestrictionsSender = routeUtils.createSender('mosaicRestriction.mosaicGlobalRestriction');
		const mosaicAddressRestrictionsSender = routeUtils.createSender('mosaicRestriction.mosaicAddressRestriction');

		server.get('/restrictions/account/:address', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			return db.accountRestrictionsByAddresses([accountAddress])
				.then(accountRestrictionsSender.sendOne(req.params.address, res, next));
		});

		server.get('/restrictions/mosaic/:mosaicId', (req, res, next) => {
			const mosaicId = routeUtils.parseArgument(req.params, 'mosaicId', uint64.fromHex);

			return db.mosaicRestrictionsByMosaicIds(
				[mosaicId],
				restriction.mosaicRestriction.restrictionType.global
			).then(mosaicGlobalRestrictionsSender.sendOne(req.params.mosaicId, res, next));
		});

		server.get('/restrictions/mosaic/:mosaicId/address/:targetAddress', (req, res, next) => {
			const mosaicId = routeUtils.parseArgument(req.params, 'mosaicId', uint64.fromHex);
			const targetAddress = routeUtils.parseArgument(req.params, 'targetAddress', 'address');

			return db.mosaicAddressRestrictions(mosaicId, [targetAddress])
				.then(mosaicAddressRestrictionsSender.sendOne(req.params.targetAddress, res, next));
		});

		server.post('/restrictions/account', (req, res, next) => {
			const addresses = routeUtils.parseArgumentAsArray(req.params, 'addresses', 'address');

			return db.accountRestrictionsByAddresses(addresses)
				.then(accountRestrictionsSender.sendArray('addresses', res, next));
		});

		server.post('/restrictions/mosaic', (req, res, next) => {
			const mosaicIds = routeUtils.parseArgumentAsArray(req.params, 'mosaicIds', uint64.fromHex);

			return db.mosaicRestrictionsByMosaicIds(
				mosaicIds,
				restriction.mosaicRestriction.restrictionType.global
			).then(mosaicGlobalRestrictionsSender.sendArray('mosaicIds', res, next));
		});

		server.post('/restrictions/mosaic/:mosaicId', (req, res, next) => {
			const mosaicId = routeUtils.parseArgument(req.params, 'mosaicId', uint64.fromHex);
			const addresses = routeUtils.parseArgumentAsArray(req.params, 'addresses', 'address');

			return db.mosaicAddressRestrictions(mosaicId, addresses)
				.then(mosaicAddressRestrictionsSender.sendArray('addresses', res, next));
		});
	}
};
