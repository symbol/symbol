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

const cmcRoutes = require('../../../src/plugins/cmc/cmcRoutes');
const cmcUtils = require('../../../src/plugins/cmc/cmcUtils');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');
const fs = require('fs');

const { uint64 } = catapult.utils;

describe('cmc routes', () => {
	describe('network currency supply', () => {
		const maxSupply = 9000000000000000;
		const XYMSupply = 8998999998000000;

		const mosaicsSample = [{
			id: '',
			mosaic: {
				id: '1234567890ABCDEF',
				supply: XYMSupply,
				startHeight: '',
				ownerAddress: '',
				revision: 1,
				flags: 3,
				divisibility: 3,
				duration: ''
			}
		}];

		const accountsSample = [{
			id: 'random1',
			account: {
				address: '',
				addressHeight: '',
				publicKey: '',
				publicKeyHeight: '',
				supplementalPublicKeys: {},
				importance: '',
				importanceHeight: '',
				activityBuckets: [],
				mosaics: [
					{ id: 0, amount: uint64.fromUint((1000000)) }
				]
			}
		}];

		const dbMosaicsFake = sinon.fake(() => Promise.resolve(mosaicsSample));
		const dbAccountsFake = sinon.fake(() => Promise.resolve(accountsSample));

		const mockServer = new MockServer();

		const db = {
			mosaicsByIds: dbMosaicsFake,
			catapultDb: {
				accountsByIds: dbAccountsFake
			}
		};

		const services = { config: { apiNode: {} } };
		cmcRoutes.register(mockServer.server, db, services);

		const req = { params: {} };

		beforeEach(() => {
			mockServer.resetStats();
			dbMosaicsFake.resetHistory();
		});

		describe('GET', () => {
			it('network currency supply circulating', () => {
				const readFileStub = sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, `[chain]\nmaxMosaicAtomicUnits = ${maxSupply}\ncurrencyMosaicId = "0x1234567890ABCDEF"`));

				const route = mockServer.getRoute('/network/currency/supply/circulating').get();

				// Arrange:
				const totalUncirculated = accountsSample.reduce((a, b) => a + parseInt(b.account.mosaics[0].amount.toString(), 10), 0);
				const circulatingSupply = XYMSupply - totalUncirculated;

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal(cmcUtils.convertToRelative(circulatingSupply));
					readFileStub.restore();
				});
			});

			it('network currency supply total', () => {
				const readFileStub = sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, '[chain]\ncurrencyMosaicId = 0x1234567890ABCDEF'));

				const route = mockServer.getRoute('/network/currency/supply/total').get();

				// Arrange:
				const xymSupply = cmcUtils.convertToRelative(mosaicsSample[0].mosaic.supply);

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal(xymSupply);
					readFileStub.restore();
				});
			});

			it('network currency supply max', () => {
				const readFileStub = sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, `[chain]\nmaxMosaicAtomicUnits = ${maxSupply}`));

				const route = mockServer.getRoute('/network/currency/supply/max').get();

				// Arrange:
				const mosaicMaxSupply = cmcUtils.convertToRelative(maxSupply);

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal(mosaicMaxSupply);
					readFileStub.restore();
				});
			});
		});
	});
});
