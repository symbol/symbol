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
const receiptsRoutes = require('../../../src/plugins/routes/receiptsRoutes');
const sinon = require('sinon');
const test = require('../../routes/utils/routeTestUtils');

const { convert } = catapult.utils;
const { expect } = require('chai');

describe('receipts routes', () => {
	describe('get transaction statements by height', () => {
		const endpointUnderTest = '/block/:height/receipts';

		const transactionStatementData = ['dummyStatement'];
		const addressResolutionStatementData = ['dummyStatement', 'dummyStatement'];
		const mosaicResolutionStatementData = ['dummyStatement'];
		const statementsStubs = [
			sinon.stub().callsFake(() => Promise.resolve(transactionStatementData)),
			sinon.stub().callsFake(() => Promise.resolve(addressResolutionStatementData)),
			sinon.stub().callsFake(() => Promise.resolve(mosaicResolutionStatementData))
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
			transactionStatementsAtHeight: statementsStubs[0],
			addressResolutionStatementsAtHeight: statementsStubs[1],
			mosaicResolutionStatementsAtHeight: statementsStubs[2]
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

		beforeEach(() => statementsStubs.forEach(stub => stub.resetHistory()));

		it('returns result if provided height is valid', () => {
			// Arrange:
			const queriedHeight = highestHeight - 10;
			const req = { params: { height: queriedHeight.toString() } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				statementsStubs.forEach((stub, index) => expect(stub.calledOnceWith(queriedHeight), `failed at index ${index}`));

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
				statementsStubs.forEach((stub, index) => expect(stub.calledOnce, `failed at index ${index}`));
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
			statementsStubs.forEach((stub, index) => expect(stub.notCalled, `failed at index ${index}`));
		});
	});

	describe('get receipts merkle path', () => {
		const endpointUnderTest = '/block/:height/receipt/:hash/merkle';

		const formatHashAsBinary = hash => test.factory.createBinary(Buffer.from(convert.hexToUint8(hash), 'hex'));
		const formatBinaryAsHash = binary => convert.uint8ToHex(binary.buffer);

		const merkleTree = [
			formatHashAsBinary('9922093F19F7160BDCBCA8AA48499DA8DF532D4102745670B85AA4BDF63B8D59'),
			formatHashAsBinary('E8FCFD95CA220D442BE748F5494001A682DC8015A152EBC433222136E99A96B8'),
			formatHashAsBinary('C1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876'),
			formatHashAsBinary('F8E8FCDAD1B94D2C76D769B113FF5CAC5D5170772F2D80E466EB04FCA23D6887'),
			formatHashAsBinary('2D3418274BBC250616223C162534B460216AED82C4FA9A87B53083B7BA7A9391'),
			formatHashAsBinary('AEAF30ED55BBE4805C53E5232D88242F0CF719F99A8E6D339BCBF5D5DE85E1FB'),
			formatHashAsBinary('AFE6C917BABA60ADC1512040CC35033B563DAFD1718FA486AB1F3D9B84866B27')
		];

		const blockInfoMockData = {
			meta: {
				numStatements: 4,
				statementMerkleTree: merkleTree
			}
		};

		const blockInfoStub = sinon.stub().callsFake(() => Promise.resolve(blockInfoMockData));

		const routes = {};
		const server = {
			get: (path, handler) => {
				routes[path] = handler;
			}
		};

		const highestHeight = 50;
		receiptsRoutes.register(server, {
			chainInfo: () => Promise.resolve({ height: highestHeight }),
			blockWithStatementMerkleTreeAtHeight: blockInfoStub
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
			blockInfoStub.resetHistory();
		});

		it('returns a merkle path for valid height and statement', () => {
			// Arrange:
			const queriedHeight = highestHeight - 10;
			const hash = formatBinaryAsHash(merkleTree[2]);
			const req = { params: { height: queriedHeight.toString(), hash } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(blockInfoStub.calledOnceWith(queriedHeight));
				expect(sentResponse).to.deep.equal({
					payload: {
						merklePath: [
							{ position: 2, hash: merkleTree[3].buffer },
							{ position: 1, hash: merkleTree[4].buffer }
						]
					},
					type: 'merkleProofInfo'
				});
			});
		});

		it('returns 404 if not found in the database', () => {
			// Arrange:
			const queriedHeight = highestHeight + 10;
			const req = { params: { height: queriedHeight.toString(), hash: formatBinaryAsHash(merkleTree[2]) } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(blockInfoStub.calledOnceWith(queriedHeight));
				expect(sentResponse.statusCode).to.equal(404);
				expect(sentResponse.message).to.equal(`no resource exists with id '${highestHeight + 10}'`);
			});
		});

		it('returns 409 if height is invalid', () => {
			// Arrange:
			const req = { params: { height: '10A', hash: formatBinaryAsHash(merkleTree[2]) } };

			// Act:
			const route = routes[endpointUnderTest];
			const apiResponse = expect(() => route(req, res, next).then(() => {})).to;

			// Assert:
			apiResponse.throw('height has an invalid format');
			apiResponse.with.property('statusCode', 409);
			apiResponse.with.property('message', 'height has an invalid format');
		});

		it('returns 409 if hash is invalid', () => {
			// Arrange:
			const req = { params: { height: highestHeight - 10, hash: 'E3F4FEAE575D81A7F728F4E3704613AF' } };

			// Act:
			const route = routes[endpointUnderTest];
			const apiResponse = expect(() => route(req, res, next).then(() => {})).to;

			// Assert:
			apiResponse.throw('hash has an invalid format');
			apiResponse.with.property('statusCode', 409);
			apiResponse.with.property('message', 'hash has an invalid format');
		});

		it('returns invalid argument if transaction is not found in the block\'s merkle tree', () => {
			// Arrange:
			const queriedHeight = highestHeight - 10;
			const hash = 'B1C1062C63CAB4197C87B366052ECE3F4FEAE575D81A7F728F4E3704613AF876';
			const req = { params: { height: queriedHeight.toString(), hash } };

			// Act:
			const route = routes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(blockInfoStub.calledOnceWith(queriedHeight));
				expect(sentResponse).to.have.property('statusCode', 409);
				expect(sentResponse).to.have.property('message', `hash '${hash}' not included in block height '${queriedHeight}'`);
			});
		});

		it('returns invalid argument if block has no statements, thus no merkle tree (hash does not belong to block)', () => {
			// Arrange:
			const blockInfoEmptyStatementMerkleStub = sinon.stub().callsFake(() => Promise.resolve({
				meta: {
					numStatements: 0,
					statementMerkleTree: []
				}
			}));

			const emptyRoutes = {};
			const emptyServer = {
				get: (path, handler) => {
					emptyRoutes[path] = handler;
				}
			};

			receiptsRoutes.register(emptyServer, {
				chainInfo: () => Promise.resolve({ height: highestHeight }),
				blockWithStatementMerkleTreeAtHeight: blockInfoEmptyStatementMerkleStub
			});

			const queriedHeight = highestHeight - 10;
			const hash = formatBinaryAsHash(merkleTree[2]);
			const req = { params: { height: queriedHeight.toString(), hash } };

			// Act:
			const route = emptyRoutes[endpointUnderTest];
			return route(req, res, next).then(() => {
				// Assert:
				expect(blockInfoEmptyStatementMerkleStub.calledOnceWith(queriedHeight));
				expect(sentResponse).to.have.property('statusCode', 409);
				expect(sentResponse).to.have.property('message', `hash '${hash}' not included in block height '${queriedHeight}'`);
			});
		});
	});
});
