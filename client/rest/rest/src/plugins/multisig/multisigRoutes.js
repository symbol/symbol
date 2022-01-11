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

const multisigUtils = require('./multisigUtils');
const merkleUtils = require('../../routes/merkleUtils');
const routeUtils = require('../../routes/routeUtils');
const catapult = require('catapult-sdk');

const { PacketType } = catapult.packet;

module.exports = {
	register: (server, db, services) => {
		server.get('/account/:address/multisig', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');

			return db.multisigsByAddresses([accountAddress])
				.then(routeUtils.createSender('multisigEntry').sendOne(req.params.address, res, next));
		});

		server.get('/account/:address/multisig/merkle', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			const state = PacketType.multisigStatePath;
			return merkleUtils.requestTree(services, state,
				accountAddress).then(response => {
				res.send(response);
				next();
			});
		});

		server.get('/account/:address/multisig/graph', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');
			return multisigUtils.getMultisigGraph(db, accountAddress)
				.then(response => {
					const sender = routeUtils.createSender('multisigGraph');
					return undefined === response
						? sender.sendOne(req.params.address, res, next)(response)
						: sender.sendArray(req.params.address, res, next)(response);
				});
		});
	}
};
