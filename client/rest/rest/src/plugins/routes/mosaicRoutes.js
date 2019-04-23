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
const dbUtils = require('../../db/dbUtils');
const routeUtils = require('../../routes/routeUtils');

const { convertToLong } = dbUtils;
const { uint64 } = catapult.utils;

const getOnlyNamespacesWithRegisterTransaction = (namespaces, transactions) =>
	namespaces.filter(n => transactions.some(t => t.transaction.namespaceId.equals(n.namespace[`level${n.namespace.depth - 1}`])));

const getUniqueTransactionsBasedOnHeight = transactions => {
	const uniqueTransactions = [];
	transactions.sort((lhs, rhs) => {
		if (lhs.meta.height.equals(rhs.meta.height))
			return rhs.meta.index > lhs.meta.index ? 1 : -1;
		return rhs.meta.height.greaterThan(lhs.meta.height) ? 1 : -1;
	});
	transactions.forEach(t => {
		if (!uniqueTransactions.some(ut => ut.namespaceId.equals(t.transaction.namespaceId))) {
			uniqueTransactions.push({
				namespaceId: t.transaction.namespaceId,
				name: t.transaction.name.value()
			});
		}
	});
	return uniqueTransactions;
};

module.exports = {
	register: (server, db) => {
		const mosaicSender = routeUtils.createSender('mosaicDescriptor');

		routeUtils.addGetPostDocumentRoutes(
			server,
			mosaicSender,
			{ base: '/mosaic', singular: 'mosaicId', plural: 'mosaicIds' },
			params => db.mosaicsByIds(params),
			uint64.fromHex
		);

		server.post('/mosaic/names', (req, res, next) => {
			const mosaicIds = routeUtils.parseArgumentAsArray(req.params, 'mosaicIds', uint64.fromHex);

			return db.activeNamespacesByMosaicsIds(mosaicIds).then(namespaces => {
				const namespaceIds = [];
				namespaces.forEach(n => {
					namespaceIds.push(n.namespace.level0);
					if (2 <= n.namespace.depth) namespaceIds.push(n.namespace.level1);
					if (3 <= n.namespace.depth) namespaceIds.push(n.namespace.level2);
				});

				return db.registerNamespaceTransactionsByNamespaceIds(namespaceIds).then(transactions => {
					const namespacesWithRegisterTransaction = getOnlyNamespacesWithRegisterTransaction(namespaces, transactions);
					const uniqueTransactions = getUniqueTransactionsBasedOnHeight(transactions);
					const namesTuples = mosaicIds.map(mosaicId => {
						const names = [];
						namespacesWithRegisterTransaction.filter(n => n.namespace.alias.mosaicId.equals(convertToLong(mosaicId)))
							.forEach(n => {
								let { name } = uniqueTransactions.find(t => t.namespaceId.equals(n.namespace.level0));
								if (2 <= n.namespace.depth)
									name += `.${uniqueTransactions.find(t => t.namespaceId.equals(n.namespace.level1)).name}`;
								if (3 <= n.namespace.depth)
									name += `.${uniqueTransactions.find(t => t.namespaceId.equals(n.namespace.level2)).name}`;
								names.push(name);
							});
						return { mosaicId, names };
					});

					res.send({ payload: namesTuples, type: 'mosaicNamesTuples' });
					return next();
				});
			});
		});
	}
};
