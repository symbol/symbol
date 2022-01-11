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

/** @module utils/future */

const future = {
	/**
	 * Makes a future retryable.
	 * @param {Function} futureSupplier Function that returns a new instance of the wrapped future.
	 * @param {Numeric} maxAttempts Maximum number of attempts.
	 * @param {Function} waitTimeSupplier Function that calculates the amount of time to wait after a failure.
	 * @returns {Promise} A promise that is resolved when the future succeeds or the maximum number of attempts have failed.
	 */
	makeRetryable: (futureSupplier, maxAttempts, waitTimeSupplier) => {
		let numRemainingAttempts = maxAttempts;
		const step = (resolve, reject) => {
			--numRemainingAttempts;
			futureSupplier()
				.then(o => {
					resolve(o);
				})
				.catch(err => {
					if (0 === numRemainingAttempts) {
						reject(err);
						return;
					}

					const waitTime = waitTimeSupplier(maxAttempts - numRemainingAttempts, err);
					setTimeout(() => { step(resolve, reject); }, waitTime);
				});
		};

		return new Promise((resolve, reject) => {
			step(resolve, reject);
		});
	}
};

module.exports = future;
