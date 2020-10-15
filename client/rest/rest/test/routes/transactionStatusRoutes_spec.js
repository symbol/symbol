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

const { MockServer } = require('./utils/routeTestUtils');
const dbFacade = require('../../src/routes/dbFacade');
const routeResultTypes = require('../../src/routes/routeResultTypes');
const routeUtils = require('../../src/routes/routeUtils');
const transactionStatusRoutes = require('../../src/routes/transactionStatusRoutes');
const catapult = require('catapult-sdk');
const { expect } = require('chai');
const sinon = require('sinon');

const { convert } = catapult.utils;

describe('transaction status routes', () => {
	describe('calls addGetPostDocumentRoutes once with correct params', () => {
		// Arrange:
		const mockServer = new MockServer();
		const db = {
			transactionsByHashesFailed: () => Promise.resolve([]),
			transactionsByHashesUnconfirmed: () => Promise.resolve([]),
			transactionsByHashes: () => Promise.resolve([])
		};
		const services = { config: { transactionStates: [] } };

		const routeInfo = {
			base: '/transactionStatus',
			singular: 'hash',
			plural: 'hashes'
		};

		let addGetPostDocumentRoutesSpy = null;
		let routeUtilsCreateSenderSpy = null;
		let transactionStatusesByHashesSpy = null;

		before(() => {
			addGetPostDocumentRoutesSpy = sinon.spy(routeUtils, 'addGetPostDocumentRoutes');
			routeUtilsCreateSenderSpy = sinon.spy(routeUtils, 'createSender');
			transactionStatusesByHashesSpy = sinon.spy(dbFacade, 'transactionStatusesByHashes');

			// Act:
			transactionStatusRoutes.register(mockServer.server, db, services);
		});

		// Assert:
		it('calls addGetPostDocumentRoutes once', () => {
			expect(addGetPostDocumentRoutesSpy.calledOnce).to.equal(true);
		});

		it('calls addGetPostDocumentRoutes with correct server', () => {
			expect(addGetPostDocumentRoutesSpy.firstCall.args[0]).to.deep.equal(mockServer.server);
		});

		it('calls addGetPostDocumentRoutes with correct sender', () => {
			expect(routeUtilsCreateSenderSpy.calledOnce).to.equal(true);
			expect(routeUtilsCreateSenderSpy.firstCall.args[0]).to.deep.equal(routeResultTypes.transactionStatus);
			expect(addGetPostDocumentRoutesSpy.firstCall.args[1]).to.deep.equal(routeUtilsCreateSenderSpy.firstCall.returnValue);
		});

		it('calls addGetPostDocumentRoutes with correct route info', () => {
			expect(addGetPostDocumentRoutesSpy.firstCall.args[2]).to.deep.equal(routeInfo);
		});

		it('calls addGetPostDocumentRoutes with correct document retriever', () => {
			const calledDocumentRetriever = addGetPostDocumentRoutesSpy.firstCall.args[3];
			const paramHashes = ['6E9D130BBBB1C3190B02AF751CBEE32BEF6D6AE045E7618E4CE4D0BD582B6A27'];

			return calledDocumentRetriever(paramHashes).then(() => {
				expect(transactionStatusesByHashesSpy.calledOnce).to.equal(true);
				expect(transactionStatusesByHashesSpy.firstCall.args[0]).to.deep.equal(db);
				expect(transactionStatusesByHashesSpy.firstCall.args[1]).to.deep.equal(paramHashes);
				expect(transactionStatusesByHashesSpy.firstCall.args[2]).to.deep.equal(services.config.transactionStates);
			});
		});

		it('calls addGetPostDocumentRoutes with correct parser', () => {
			const calledParser = addGetPostDocumentRoutesSpy.firstCall.args[4];

			// - invalid length of hash
			expect(() => { calledParser('abcd'); }).to.throw('invalid length of hash \'4\'');

			// - valid length of hash
			const hexValue = '6BAD46BDBEF2B84D03BA9668E635EF14FA66099258FE669DADCF8C23324C5DF1';
			const parsedValue = convert.hexToUint8(hexValue);
			expect(calledParser(hexValue)).to.deep.equal(parsedValue);
		});

		after(() => {
			addGetPostDocumentRoutesSpy.restore();
			routeUtilsCreateSenderSpy.restore();
			transactionStatusesByHashesSpy.restore();
		});
	});
});
