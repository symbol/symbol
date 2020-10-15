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

const namespaceUtils = {
	/**
	 * Returns function for processing alias names requests.
	 * @param {module:db/CatapultDb} catapultDb Catapult database.
	 * @param {numeric} aliasType Alias type.
	 * @param {Function} getParams Function to parse request params into ids.
	 * @param {Function} namespaceFilter Function to filter namespaces based on ids.
	 * @param {string} aliasFieldName Alias field name to show in the results.
	 * @param {string} schemaName Schema name to parse results.
	 * @returns {Function} Restify response function to process alias names requests.
	 */
	aliasNamesRoutesProcessor: (
		catapultDb,
		aliasType,
		getParams,
		namespaceFilter,
		aliasFieldName,
		schemaName
	) => (req, res, next) => {
		const getNewestTransactions = transactions => {
			const uniqueTransactions = [];
			transactions.sort((lhs, rhs) => {
				if (lhs.meta.height.equals(rhs.meta.height))
					return rhs.meta.index > lhs.meta.index ? 1 : -1;
				return rhs.meta.height.greaterThan(lhs.meta.height) ? 1 : -1;
			});
			transactions.forEach(t => {
				if (!uniqueTransactions.some(ut => ut.namespaceId.equals(t.transaction.id))) {
					uniqueTransactions.push({
						namespaceId: t.transaction.id,
						name: t.transaction.name.value()
					});
				}
			});
			return uniqueTransactions;
		};

		const getOnlyNamespacesWithRegisterTransaction = (namespaces, transactions) =>
			namespaces.filter(n => transactions
				.some(t => t.transaction.id.equals(n.namespace[`level${n.namespace.depth - 1}`])));

		const ids = getParams(req);

		return catapultDb.activeNamespacesWithAlias(aliasType, ids).then(namespaces => {
			const namespaceIds = [];
			namespaces.forEach(n => {
				namespaceIds.push(n.namespace.level0);
				if (2 <= n.namespace.depth)
					namespaceIds.push(n.namespace.level1);
				if (3 <= n.namespace.depth)
					namespaceIds.push(n.namespace.level2);
			});

			return catapultDb.registerNamespaceTransactionsByNamespaceIds(namespaceIds).then(transactions => {
				const namespacesWithRegisterTransaction = getOnlyNamespacesWithRegisterTransaction(namespaces, transactions);
				const uniqueTransactions = getNewestTransactions(transactions);
				const namesTuples = ids.map(id => {
					let aliasName;
					const names = [];
					namespacesWithRegisterTransaction.filter(n => namespaceFilter(n, id))
						.forEach(n => {
							aliasName = uniqueTransactions.find(t => t.namespaceId.equals(n.namespace.level0)).name;
							if (2 <= n.namespace.depth)
								aliasName += `.${uniqueTransactions.find(t => t.namespaceId.equals(n.namespace.level1)).name}`;
							if (3 <= n.namespace.depth)
								aliasName += `.${uniqueTransactions.find(t => t.namespaceId.equals(n.namespace.level2)).name}`;
							names.push(aliasName);
						});
					return { [aliasFieldName]: id, names };
				});

				res.send({ payload: { [schemaName]: namesTuples }, type: schemaName });

				return next();
			});
		});
	}
};

module.exports = namespaceUtils;
