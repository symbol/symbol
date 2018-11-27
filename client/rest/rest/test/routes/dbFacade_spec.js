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

const dbFacade = require('../../src/routes/dbFacade');
const { expect } = require('chai');

describe('db facade', () => {
	describe('run height dependent operation', () => {
		const runHeightDependentOperationTest = (requestHeight, chainHeight, isRequestValid) => {
			// Arrange:
			const db = { chainInfo: () => Promise.resolve({ height: chainHeight }) };

			// Act:
			return dbFacade.runHeightDependentOperation(db, requestHeight, () => Promise.resolve(17))
				.then(result => {
					expect(result).to.deep.equal({
						isRequestValid,
						payload: isRequestValid ? 17 : undefined
					});
				});
		};

		it('returns operation result when request height is less than chain height', () => runHeightDependentOperationTest(3, 10, true));
		it('returns operation result when request height is equal to chain height', () => runHeightDependentOperationTest(10, 10, true));
		it('returns undefined when request height is greater than chain height', () => runHeightDependentOperationTest(11, 10, false));
	});

	describe('transaction statuses by hashes', () => {
		const createHandler = (dbApiName, db, collected, traits) => {
			collected[dbApiName] = [];
			db[dbApiName] = hashes => {
				collected[dbApiName].push(hashes);
				return Promise.resolve(traits);
			};
		};

		const addTransactionStatusesByHashesTest = traits => {
			// Arrange:
			const collected = {};
			const db = {};
			createHandler('transactionsByHashesFailed', db, collected, traits.failed || []);
			createHandler('transactionsByHashesUnconfirmed', db, collected, traits.unconfirmed || []);
			createHandler('transactionsByHashes', db, collected, traits.confirmed || []);
			createHandler('transactionsByHashesCustom', db, collected, traits.custom || []);

			// Act:
			const hashes = [1, 2, 3, 4];
			const transactionStates = [{ dbPostfix: 'Custom', friendlyName: 'custom' }];
			return dbFacade.transactionStatusesByHashes(db, hashes, transactionStates).then(result => {
				// Assert:
				Object.keys(collected).forEach(name => {
					const capturedHashes = collected[name];
					expect(capturedHashes.length, `${name} handler collected invalid number of elements`).to.equal(1);
					// note: copying hashes would be unexpected, so deliberately make a shallow comparison
					expect(capturedHashes[0], `${name} handler collected invalid hashes`).to.equal(hashes);
				});

				expect(result).to.deep.equal(traits.expected);
			});
		};

		const createFailed = value => ({ f: value });
		const createFailedStatus = value => ({ group: 'failed', f: value });
		const createTransaction = (hash, deadline, height) => ({ meta: { hash, height }, transaction: { deadline } });
		const createUnconfirmedStatus = (hash, deadline) => ({
			group: 'unconfirmed', status: 0, hash, deadline, height: 0
		});
		const createConfirmedStatus = (hash, deadline, height) => ({
			group: 'confirmed', status: 0, hash, deadline, height
		});
		const createCustomStatus = (hash, deadline, height) => ({
			group: 'custom', status: 0, hash, deadline, height
		});

		it('unknown hashes are properly mapped', () =>
			addTransactionStatusesByHashesTest({
				expected: []
			}));

		it('failed transactions have type appended', () =>
			addTransactionStatusesByHashesTest({
				failed: [createFailed(123), createFailed(456)],
				expected: [createFailedStatus(123), createFailedStatus(456)]
			}));

		it('unconfirmed transactions are properly mapped', () =>
			addTransactionStatusesByHashesTest({
				unconfirmed: [createTransaction(111, 222, 0), createTransaction(333, 444, 0)],
				expected: [createUnconfirmedStatus(111, 222), createUnconfirmedStatus(333, 444)]
			}));

		it('confirmed transactions are properly mapped', () =>
			addTransactionStatusesByHashesTest({
				confirmed: [createTransaction(55, 66, 77), createTransaction(88, 99, 11)],
				expected: [createConfirmedStatus(55, 66, 77), createConfirmedStatus(88, 99, 11)]
			}));

		it('custom transactions are properly mapped', () =>
			addTransactionStatusesByHashesTest({
				custom: [createTransaction(87, 98, 43), createTransaction(34, 89, 22)],
				expected: [createCustomStatus(87, 98, 43), createCustomStatus(34, 89, 22)]
			}));

		it('mixed elements are properly mapped', () =>
			addTransactionStatusesByHashesTest({
				failed: [createFailedStatus(123), createFailedStatus(456)],
				unconfirmed: [createTransaction(111, 222, 0), createTransaction(333, 444, 0)],
				confirmed: [createTransaction(55, 66, 77), createTransaction(88, 99, 11)],
				custom: [createTransaction(87, 98, 43)],
				expected: [
					createFailedStatus(123), createFailedStatus(456),
					createUnconfirmedStatus(111, 222), createUnconfirmedStatus(333, 444),
					createCustomStatus(87, 98, 43),
					createConfirmedStatus(55, 66, 77), createConfirmedStatus(88, 99, 11)
				]
			}));
	});
});
