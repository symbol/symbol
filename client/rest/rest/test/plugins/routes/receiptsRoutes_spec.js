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
const routeUtils = require('../../../src/routes/routeUtils');
const sinon = require('sinon');

const { expect } = require('chai');

describe('receipts routes', () => {
	describe('get transaction statements by height', () => {
		const endpointUnderTest = '/block/:height/receipts';

		const highestHeight = 50;
		const correctQueriedHeight = highestHeight - 10;

		const transactionStatementData = ['dummyStatement'];
		const addressResolutionStatementData = ['dummyStatement', 'dummyStatement'];
		const mosaicResolutionStatementData = ['dummyStatement'];
		const statementsFake = sinon.stub();
		const orderedStatementsCollections = ['transactionStatements', 'addressResolutionStatements', 'mosaicResolutionStatements'];
		statementsFake.withArgs(correctQueriedHeight, orderedStatementsCollections[0]).returns(transactionStatementData);
		statementsFake.withArgs(correctQueriedHeight, orderedStatementsCollections[1]).returns(addressResolutionStatementData);
		statementsFake.withArgs(correctQueriedHeight, orderedStatementsCollections[2]).returns(mosaicResolutionStatementData);

		const routes = {};
		const server = {
			get: (path, handler) => {
				routes[path] = handler;
			}
		};

		receiptsRoutes.register(server, {
			catapultDb: {
				chainInfo: () => Promise.resolve({ height: highestHeight })
			},
			statementsAtHeight: statementsFake
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

		beforeEach(() => statementsFake.resetHistory());

		it('returns result if provided height is valid', () => {
			// Arrange:
			const req = { params: { height: correctQueriedHeight.toString() } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(statementsFake.calledThrice).to.equal(true);
				orderedStatementsCollections.forEach((statementCollection, index) => expect(
					statementsFake.calledWith(correctQueriedHeight, statementCollection),
					`failed at index ${index}`
				).to.equal(true));

				expect(sentResponse).to.deep.equal({
					payload: {
						transactionStatements: transactionStatementData,
						addressResolutionStatements: addressResolutionStatementData,
						mosaicResolutionStatements: mosaicResolutionStatementData
					},
					type: 'receipts'
				});
			});
		});

		it('returns 404 if not found in the database', () => {
			// Arrange:
			const queriedHeight = highestHeight + 10;
			const req = { params: { height: queriedHeight.toString() } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(statementsFake.calledThrice).to.equal(true);
				expect(sentResponse.statusCode).to.equal(404);
				expect(sentResponse.message).to.equal(`no resource exists with id '${highestHeight + 10}'`);
			});
		});

		it('returns 409 if height is invalid', () => {
			// Arrange:
			const req = { params: { height: '10A' } };

			// Act:
			const route = routes[endpointUnderTest];
			const apiResponse = expect(() => route(req, res, next).then(() => {})).to;

			// Assert:
			apiResponse.throw('height has an invalid format');
			apiResponse.with.property('statusCode', 409);
			apiResponse.with.property('message', 'height has an invalid format');
			expect(statementsFake.notCalled).to.equal(true);
		});
	});

	describe('get receipts merkle path', () => {
		it('calls blockRouteMerkleProcessor with correct params', () => {
			// Arrange:
			const blockRouteMerkleProcessorSpy = sinon.spy(routeUtils, 'blockRouteMerkleProcessor');
			const routes = {};
			const server = {
				get: (path, handler) => {
					routes[path] = handler;
				}
			};

			// Act:
			receiptsRoutes.register(server, {}, {});

			// Assert:
			expect(blockRouteMerkleProcessorSpy.calledOnce).to.equal(true);
			expect(blockRouteMerkleProcessorSpy.firstCall.args[1]).to.equal('numStatements');
			expect(blockRouteMerkleProcessorSpy.firstCall.args[2]).to.equal('statementMerkleTree');
			blockRouteMerkleProcessorSpy.restore();
		});
	});
});
