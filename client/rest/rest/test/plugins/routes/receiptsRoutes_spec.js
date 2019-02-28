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

		const transactionStatementData = ['dummyStatement'];
		const addressResolutionStatementData = ['dummyStatement', 'dummyStatement'];
		const mosaicResolutionStatementData = ['dummyStatement'];
		const statementsFakes = [
			sinon.fake(() => Promise.resolve(transactionStatementData)),
			sinon.fake(() => Promise.resolve(addressResolutionStatementData)),
			sinon.fake(() => Promise.resolve(mosaicResolutionStatementData))
		];

		const routes = {};
		const server = {
			get: (path, handler) => {
				routes[path] = handler;
			}
		};

		const highestHeight = 50;
		receiptsRoutes.register(server, {
			chainInfo: () => Promise.resolve({ height: highestHeight }),
			transactionStatementsAtHeight: statementsFakes[0],
			addressResolutionStatementsAtHeight: statementsFakes[1],
			mosaicResolutionStatementsAtHeight: statementsFakes[2]
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

		beforeEach(() => statementsFakes.forEach(fake => fake.resetHistory()));

		it('returns result if provided height is valid', () => {
			// Arrange:
			const queriedHeight = highestHeight - 10;
			const req = { params: { height: queriedHeight.toString() } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				statementsFakes.forEach((fake, index) => expect(
					fake.calledOnceWith(queriedHeight),
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
				statementsFakes.forEach((fake, index) => expect(fake.calledOnce, `failed at index ${index}`));
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
			statementsFakes.forEach((fake, index) => expect(fake.notCalled, `failed at index ${index}`));
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
			expect(blockRouteMerkleProcessorSpy.firstCall.args[2]).to.equal('numStatements');
			expect(blockRouteMerkleProcessorSpy.firstCall.args[3]).to.equal('statementMerkleTree');
			blockRouteMerkleProcessorSpy.restore();
		});
	});
});
