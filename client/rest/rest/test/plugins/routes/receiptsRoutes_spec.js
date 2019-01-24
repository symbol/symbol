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

const receiptsRoutes = require('../../../src/plugins/routes/receiptsRoutes');
const sinon = require('sinon');
const { expect } = require('chai');

describe('receipts routes', () => {
	describe('get transaction statements by height', () => {
		const transactionStatementData = { dummyData1: 'dummyData1' };
		const addressResolutionStatementData = { dummyData2: 'dummyData2' };
		const mosaicResolutionStatementData = { dummyData3: 'dummyData3' };
		const transactionStatementsAtHeightStub = sinon.stub().callsFake(() => Promise.resolve(transactionStatementData));
		const addressResolutionStatementsAtHeightStub = sinon.stub().callsFake(() => Promise.resolve(addressResolutionStatementData));
		const mosaicResolutionStatementsAtHeightStub = sinon.stub().callsFake(() => Promise.resolve(mosaicResolutionStatementData));

		const routes = {};
		const server = {
			get: (path, handler) => {
				routes[path] = handler;
			}
		};

		const highestHeight = 50;
		receiptsRoutes.register(server, {
			chainInfo: () => Promise.resolve({ height: highestHeight }),
			transactionStatementsAtHeight: transactionStatementsAtHeightStub,
			addressResolutionStatementsAtHeight: addressResolutionStatementsAtHeightStub,
			mosaicResolutionStatementsAtHeight: mosaicResolutionStatementsAtHeightStub
		});

		let sentResponse;
		const next = () => {};
		const res = {
			send: response => {
				sentResponse = response;
			},
			redirect: () => {
				next();
			}
		};

		beforeEach(() => {
			transactionStatementsAtHeightStub.resetHistory();
			addressResolutionStatementsAtHeightStub.resetHistory();
			mosaicResolutionStatementsAtHeightStub.resetHistory();
		});

		it('returns result if provided height is valid', done => {
			// Arrange:
			const queriedHeight = highestHeight - 10;
			const req = { params: { height: (queriedHeight).toString() } };

			// Act:
			const route = routes['/block/:height/receipts'];
			route(req, res, next).then(() => {
				// Assert:
				expect(transactionStatementsAtHeightStub.calledOnceWith(queriedHeight)).to.equal(true);
				expect(addressResolutionStatementsAtHeightStub.calledOnceWith(queriedHeight)).to.equal(true);
				expect(mosaicResolutionStatementsAtHeightStub.calledOnceWith(queriedHeight)).to.equal(true);
				expect(sentResponse).to.deep.equal({
					payload: {
						transactionStatements: transactionStatementData,
						addressResolutionStatements: addressResolutionStatementData,
						mosaicResolutionStatements: mosaicResolutionStatementData
					},
					type: 'receipts'
				});
				done();
			});
		});

		it('returns 404 if not found in the database', done => {
			// Arrange:
			const queriedHeight = highestHeight + 10;
			const req = { params: { height: (queriedHeight).toString() } };

			// Act:
			const route = routes['/block/:height/receipts'];
			route(req, res, next).then(() => {
				// Assert:
				expect(transactionStatementsAtHeightStub.calledOnce).to.equal(true);
				expect(addressResolutionStatementsAtHeightStub.calledOnce).to.equal(true);
				expect(mosaicResolutionStatementsAtHeightStub.calledOnce).to.equal(true);
				expect(sentResponse.statusCode).to.equal(404);
				expect(sentResponse.message).to.equal(`no resource exists with id '${highestHeight + 10}'`);
				done();
			});
		});

		it('returns 409 if height is invalid', done => {
			// Arrange:
			const req = { params: { height: '10A' } };

			// Act:
			const route = routes['/block/:height/receipts'];
			const apiResponse = expect(() => route(req, res, next).then(() => {})).to;

			// Assert:
			apiResponse.throw('height has an invalid format');
			apiResponse.with.property('statusCode', 409);
			apiResponse.with.deep.property('message', 'height has an invalid format');
			expect(transactionStatementsAtHeightStub.called).to.equal(false);
			expect(addressResolutionStatementsAtHeightStub.called).to.equal(false);
			expect(mosaicResolutionStatementsAtHeightStub.called).to.equal(false);
			done();
		});
	});
});
