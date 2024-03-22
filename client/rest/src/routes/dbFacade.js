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

const { convertToLong } = require('../db/dbUtils');

const extractFromMetadata = (group, transaction) => ({
	group,
	code: 0,
	hash: transaction.meta.hash,
	deadline: transaction.transaction.deadline,
	height: transaction.meta.height
});

const dbFacade = {

	/**
	 * Runs a database operation that is dependent on current chain height.
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {number} height Request height.
	 * @param {Function} operation Height-dependent operation that returns a promise.
	 * @returns {object} Operation result if the request height is no greater than the chain height, undefined otherwise.
	 */
	runHeightDependentOperation: (db, height, operation) => {
		// notice that both the chain height and height-dependent operation are started at the same time in order to
		// optimize the common case when the request height is valid
		const chainStatisticPromise = db.chainStatisticCurrent();
		const operationPromise = operation();

		return Promise.all([chainStatisticPromise, operationPromise]).then(results => {
			const chainStatistic = results[0];
			const isRequestValid = convertToLong(height).lessThanOrEqual(chainStatistic.height);
			return {
				isRequestValid,
				payload: isRequestValid ? results[1] : undefined
			};
		});
	},

	/**
	 * Retrieves transaction statuses by specified hashes.
	 * @param {module:db/CatapultDb} db Catapult database.
	 * @param {Array<Uint8Array>} hashes Hashes of transactions to query.
	 * @param {Array<module:plugins/CatapultRestPlugin~TransactionStateDescriptor>} additionalTransactionStates
	 *        Additional transaction states.
	 * @returns {Promise<Array<object>>} Array of failed, unconfirmed and confirmed transactions.
	 */
	transactionStatusesByHashes: (db, hashes, additionalTransactionStates) => {
		const transactionStates = [].concat(
			// order matters and it determines the priority at which the status will be returned
			// * this was agreed by the team, since a transaction with the same hash (same transaction), can be announced multiple times
			// * and be present in different status collections at the same time also not sure if the same hash can be "failed" multiple
			// * times for different reasons (codes) in the database
			[{ dbPostfix: '', friendlyName: 'confirmed' }],
			[{ dbPostfix: 'Unconfirmed', friendlyName: 'unconfirmed' }],
			additionalTransactionStates
		);

		const promises = [];
		transactionStates.forEach(state => {
			const dbPromise = db.transactionsByHashes(state.friendlyName, hashes);
			promises.push(dbPromise.then(objs => objs.map(transaction => extractFromMetadata(state.friendlyName, transaction))));
		});
		promises.push(db.transactionsByHashesFailed(hashes)
			.then(objs => objs.map(status => status.status)) // removes wrapping property
			.then(objs => objs.map(status => Object.assign(status, { group: 'failed' }))));

		return Promise.all(promises).then(tuple => {
			const resultingStatuses = [];
			tuple.forEach(statusArray => statusArray.forEach(statusResult => {
				if (!resultingStatuses.some(status => 0 === status.hash.buffer.compare(statusResult.hash.buffer)))
					resultingStatuses.push(statusResult);
			}));
			return resultingStatuses;
		});
	}
};

module.exports = dbFacade;
