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

import AccountIdentifier from '../../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import Amount from '../../../../src/plugins/rosetta/openApi/model/Amount.js';
import Currency from '../../../../src/plugins/rosetta/openApi/model/Currency.js';
import RosettaApiError from '../../../../src/plugins/rosetta/openApi/model/Error.js';
import Operation from '../../../../src/plugins/rosetta/openApi/model/Operation.js';
import OperationIdentifier from '../../../../src/plugins/rosetta/openApi/model/OperationIdentifier.js';
import MockServer from '../../../routes/utils/MockServer.js';
import { expect } from 'chai';
import sinon from 'sinon';

// region BasicFetchStubHelper

export const BasicFetchStubHelper = {
	stubPost: (urlPath, ok, jsonResult, expectedRequestOptions = undefined, callNumber = undefined) => {
		if (!global.fetch.restore)
			sinon.stub(global, 'fetch');

		const proxy = global.fetch.withArgs(`http://localhost:3456/${urlPath}`, expectedRequestOptions || sinon.match.any);
		const result = Promise.resolve({
			ok,
			json: () => jsonResult
		});

		if (callNumber)
			proxy[`on${callNumber}Call`]().returns(result);
		else
			proxy.returns(result);
	},

	registerStubCleanup: () => {
		afterEach(() => {
			if (global.fetch.restore)
				global.fetch.restore();
		});
	}
};

// endregion

// region RosettaOperationFactory

export const RosettaOperationFactory = {
	createCurrency: (currencyName, currencyDecimals, mosaicId) => {
		const currency = new Currency(currencyName, currencyDecimals);
		if (mosaicId)
			currency.metadata = { id: mosaicId };

		return currency;
	},

	createTransferOperation: (index, address, amount, currencyName, currencyDecimals, mosaicId = undefined) => {
		const currency = RosettaOperationFactory.createCurrency(currencyName, currencyDecimals, mosaicId);

		const operation = new Operation(new OperationIdentifier(index), 'transfer');
		operation.account = new AccountIdentifier(address);
		operation.amount = new Amount(amount, currency);
		return operation;
	},

	createMultisigOperation: (index, address, metadata) => {
		const operation = new Operation(new OperationIdentifier(index), 'multisig');
		operation.account = new AccountIdentifier(address);
		operation.metadata = {
			addressAdditions: [],
			addressDeletions: [],
			...metadata
		};
		return operation;
	},

	createCosignOperation: (index, address) => {
		const operation = new Operation(new OperationIdentifier(index), 'cosign');
		operation.account = new AccountIdentifier(address);
		return operation;
	},

	setOperationStatus: operation => {
		operation.status = 'success';
		return operation;
	}
};

// endregion

// region asserts

const createMockServer = registerRoutes => {
	const mockServer = new MockServer();
	registerRoutes(mockServer.server);
	return mockServer;
};

export const assertRosettaErrorRaisedBasicWithRegister = async (registerRoutes, routeName, request, expectedError, malformRequest) => {
	// Arrange:
	const mockServer = createMockServer(registerRoutes);
	const route = mockServer.getRoute(routeName).post();

	malformRequest(request);

	// Act:
	await mockServer.callRoute(route, { body: request });

	// Assert:
	expect(mockServer.send.calledOnce).to.equal(true);
	expect(mockServer.next.calledOnce).to.equal(true);
	expect(mockServer.res.statusCode).to.equal(500);

	const response = mockServer.send.firstCall.args[0];
	expect(response).to.deep.equal(expectedError.apiError);
	expect(() => RosettaApiError.validateJSON(response)).to.not.throw();
};

export const assertRosettaSuccessBasicWithRegister = async (registerRoutes, routeName, request, expectedResponse, compareOptions = {}) => {
	// Arrange:
	const mockServer = createMockServer(registerRoutes);
	const route = mockServer.getRoute(routeName).post();

	// Act:
	await mockServer.callRoute(route, { body: request });

	// Assert:
	expect(mockServer.send.calledOnce).to.equal(true);
	expect(mockServer.next.calledOnce).to.equal(true);
	expect(mockServer.res.statusCode).to.equal(undefined);

	const response = mockServer.send.firstCall.args[0];
	if (compareOptions.roundtripJson)
		expect(JSON.parse(JSON.stringify(response))).to.deep.equal(JSON.parse(JSON.stringify(expectedResponse)));
	else
		expect(response).to.deep.equal(expectedResponse);

	expect(() => expectedResponse.constructor.validateJSON(response)).to.not.throw();
};

// endregion
