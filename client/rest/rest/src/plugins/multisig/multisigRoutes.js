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

const routeUtils = require('../../routes/routeUtils');

module.exports = {
	register: (server, db) => {
		server.get('/account/:address/multisig', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');

			return db.multisigsByAddresses([accountAddress])
				.then(routeUtils.createSender('multisigEntry').sendOne(req.params.address, res, next));
		});

		const getMultisigEntries = (multisigEntries, fieldName) => {
			const addresses = new Set();
			multisigEntries.forEach(multisigEntry => multisigEntry.multisig[fieldName].forEach(address => {
				addresses.add(address.buffer);
			}));

			return db.multisigsByAddresses(Array.from(addresses));
		};

		server.get('/account/:address/multisig/graph', (req, res, next) => {
			const accountAddress = routeUtils.parseArgument(req.params, 'address', 'address');

			const multisigLevels = [];
			return db.multisigsByAddresses([accountAddress])
				.then(multisigEntries => {
					if (0 === multisigEntries.length)
						return Promise.resolve(undefined);

					multisigLevels.push({
						level: 0,
						multisigEntries: [multisigEntries[0]]
					});

					return Promise.resolve(multisigEntries[0]);
				})
				.then(multisigEntry => {
					if (undefined === multisigEntry)
						return Promise.resolve(undefined);

					const handleUpstream = (level, multisigEntries) => getMultisigEntries(multisigEntries, 'multisigAddresses')
						.then(entries => {
							if (0 === entries.length)
								return Promise.resolve();

							multisigLevels.unshift({ level, multisigEntries: entries });
							return handleUpstream(level - 1, entries);
						});

					const handleDownstream = (level, multisigEntries) => getMultisigEntries(multisigEntries, 'cosignatoryAddresses')
						.then(entries => {
							if (0 === entries.length)
								return Promise.resolve();

							multisigLevels.push({ level, multisigEntries: entries });
							return handleDownstream(level + 1, entries);
						});

					const upstreamPromise = handleUpstream(-1, [multisigEntry]);
					const downstreamPromise = handleDownstream(1, [multisigEntry]);
					return Promise.all([upstreamPromise, downstreamPromise])
						.then(() => multisigLevels);
				})
				.then(response => {
					const sender = routeUtils.createSender('multisigGraph');
					return undefined === response
						? sender.sendOne(req.params.address, res, next)(response)
						: sender.sendArray(req.params.address, res, next)(response);
				});
		});
	}
};
