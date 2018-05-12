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

const extractFromMetadata = (group, transaction) => ({
	group,
	status: 0,
	hash: transaction.meta.hash,
	deadline: transaction.transaction.deadline,
	height: transaction.meta.height
});

module.exports = {

	/**
	 * Runs a database operation that is dependent on current chain height.
	 * @param {module:db/CatapultDb} db The catapult database.
	 * @param {numeric} height The request height.
	 * @param {function} operation The height-dependent operation that returns a promise.
	 * @returns {object} The operation result if the request height is no greater than the chain height, undefined otherwise.
	 */
	runHeightDependentOperation: (db, height, operation) => {
		// notice that both the chain height and height-dependent operation are started at the same time in order to
		// optimize the common case when the request height is valid
		const chainInfoPromise = db.chainInfo();
		const operationPromise = operation();

		return Promise.all([chainInfoPromise, operationPromise]).then(results => {
			const chainInfo = results[0];
			const isRequestValid = height <= chainInfo.height;
			return {
				isRequestValid,
				payload: isRequestValid ? results[1] : undefined
			};
		});
	},

	/**
	 * Retrieves transaction statuses by specified hashes.
	 * @param {module:db/CatapultDb} db The catapult database.
	 * @param {array} hashes Hashes of transactions to query.
	 * @param {array} additionalTransactionStates Additional transaction states.
	 * @returns {Promise.<array>} Array of failed, unconfirmed and confirmed transactions.
	 */
	transactionStatusesByHashes: (db, hashes, additionalTransactionStates) => {
		const transactionStates = [].concat(
			[{ dbPostfix: 'Unconfirmed', friendlyName: 'unconfirmed' }],
			additionalTransactionStates,
			[{ dbPostfix: '', friendlyName: 'confirmed' }]
		);

		const promises = [];
		promises.push(db.transactionsByHashesFailed(hashes).then(objs => objs.map(status => Object.assign(status, { group: 'failed' }))));
		transactionStates.forEach(state => {
			const dbPromise = db[`transactionsByHashes${state.dbPostfix}`](hashes);
			promises.push(dbPromise.then(objs => objs.map(transaction => extractFromMetadata(state.friendlyName, transaction))));
		});

		return Promise.all(promises).then(tuple => [].concat(...tuple));
	}
};
