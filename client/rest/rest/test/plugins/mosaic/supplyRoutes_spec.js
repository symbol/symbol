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

const { convertToLong } = require('../../../src/db/dbUtils');
const supplyRoutes = require('../../../src/plugins/mosaic/supplyRoutes');
const { MockServer } = require('../../routes/utils/routeTestUtils');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');
const fs = require('fs');

describe('supply routes', () => {
	describe('network currency supply', () => {
		const maxSupply = 9000000000000000;
		const xymSupply = 8998999998000000;

		const currencyMosaicId = '0x1234\'5678\'ABCD\'EF01';
		const nemesisSignerPublicKey = 'AF1B9DCF4FAD2CDC2C04B4F9CBDF3C9C884A9F05B40A59E233681E282DC824D9';
		const uncirculatingAccountPublicKey1 = 'D4912C4CA33F608E95B9C3ABAE59263B99E2DF6E87252D61F8DEFADF7DFFC455';
		const uncirculatingAccountPublicKey2 = '3BF1E1F3072E3BE0CD851E4741E101E33DB19C163895F69AA890E7CF177C878C';
		const circulatingAccountPublicKey1 = '346AA758B2ED98923204D7361A5A47C7B569C594C7904461C67459703D7B5874';

		const mosaicsSample = [{
			id: '',
			mosaic: {
				id: convertToLong([0xABCDEF01, 0x12345678]),
				supply: convertToLong(xymSupply),
				startHeight: '',
				ownerAddress: '',
				revision: 1,
				flags: 3,
				divisibility: 3,
				duration: ''
			}
		}];

		const createAccountSample = (publicKey, currencyAmount, otherAmount) => ({
			address: '',
			addressHeight: '',
			publicKey: catapult.utils.convert.hexToUint8(publicKey),
			publicKeyHeight: '',
			supplementalPublicKeys: {},
			importance: '',
			importanceHeight: '',
			activityBuckets: [],
			mosaics: [
				{ id: convertToLong([0xABCDEF01, 0x22222222]), amount: convertToLong(otherAmount) },
				{ id: convertToLong([0xABCDEF01, 0x12345678]), amount: convertToLong(currencyAmount) }
			]
		});

		const accountsSample = [
			{ id: 'random1', account: createAccountSample(nemesisSignerPublicKey, 1000000, 9000000) },
			{ id: 'random2', account: createAccountSample(uncirculatingAccountPublicKey1, 2000000, 9000000) },
			{ id: 'random3', account: createAccountSample(circulatingAccountPublicKey1, 4000000, 9000000) },
			{ id: 'random4', account: createAccountSample(uncirculatingAccountPublicKey2, 8000000, 9000000) }
		];

		const dbMosaicsFake = sinon.fake(() => Promise.resolve(mosaicsSample));
		const dbAccountsFake = sinon.fake(accountIds => {
			const filteredAccountsSample = accountsSample.filter(accountSample =>
				accountIds.some(accountId => catapult.utils.array.deepEqual(accountId.publicKey, accountSample.account.publicKey)));
			return Promise.resolve(filteredAccountsSample);
		});

		const mockServer = new MockServer();

		const db = {
			mosaicsByIds: dbMosaicsFake,
			catapultDb: {
				accountsByIds: dbAccountsFake
			}
		};

		const services = {
			config: {
				apiNode: {},
				uncirculatingAccountPublicKeys: [uncirculatingAccountPublicKey1, uncirculatingAccountPublicKey2]
			}
		};
		supplyRoutes.register(mockServer.server, db, services);

		const req = { params: {} };

		afterEach(() => {
			mockServer.resetStats();
			dbMosaicsFake.resetHistory();
			fs.readFile.restore();
		});

		describe('GET', () => {
			// Arrange:
			it('network currency supply circulating (without burns)', () => {
				sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, [
						'[network]',
						`nemesisSignerPublicKey=${nemesisSignerPublicKey}`,
						'',
						'[chain]',
						'currencyMosaicId = 0x1234\'5678\'ABCD\'EF02'
					].join('\n')));
				const route = mockServer.getRoute('/network/currency/supply/circulating').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal('8998999998000.000');
				});
			});

			it('network currency supply circulating (with burns)', () => {
				// Arrange:
				sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, [
						'[network]',
						`nemesisSignerPublicKey=${nemesisSignerPublicKey}`,
						'',
						'[chain]',
						`currencyMosaicId = ${currencyMosaicId}`
					].join('\n')));
				const route = mockServer.getRoute('/network/currency/supply/circulating').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal('8998999987000.000');
				});
			});

			it('network currency supply total', () => {
				// Arrange:
				sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, `[chain]\ncurrencyMosaicId = ${currencyMosaicId}`));

				const route = mockServer.getRoute('/network/currency/supply/total').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal('8998999998000.000');
				});
			});

			it('network currency supply max', () => {
				// Arrange:
				sinon.stub(fs, 'readFile').callsFake((path, data, callback) =>
					callback(null, `[chain]\ncurrencyMosaicId = ${currencyMosaicId}\nmaxMosaicAtomicUnits = ${maxSupply}`));

				const route = mockServer.getRoute('/network/currency/supply/max').get();

				// Act:
				return mockServer.callRoute(route, req).then(() => {
					// Assert:
					expect(mockServer.next.calledOnce).to.equal(true);
					expect(mockServer.send.firstCall.args[0]).to.equal('9000000000000.000');
				});
			});
		});
	});
});
