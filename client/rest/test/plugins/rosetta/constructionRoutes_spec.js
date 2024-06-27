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

import constructionRoutes from '../../../src/plugins/rosetta/constructionRoutes.js';
import AccountIdentifier from '../../../src/plugins/rosetta/openApi/model/AccountIdentifier.js';
import ConstructionDeriveResponse from '../../../src/plugins/rosetta/openApi/model/ConstructionDeriveResponse.js';
import RosettaError from '../../../src/plugins/rosetta/openApi/model/Error.js';
import { errors } from '../../../src/plugins/rosetta/rosettaUtils.js';
import MockServer from '../../routes/utils/MockServer.js';
import { expect } from 'chai';

describe('construction routes', () => {
	const createMockServer = () => {
		const mockServer = new MockServer();
		constructionRoutes.register(mockServer.server, {}, { config: { network: { name: 'testnet' } } });
		return mockServer;
	};

	describe('derive', () => {
		const createValidRequest = () => ({
			network_identifier: {
				blockchain: 'Symbol',
				network: 'testnet'
			},
			public_key: {
				hex_bytes: 'E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79',
				curve_type: 'edwards25519'
			}
		});

		const assertRosettaErrorRaised = (expectedError, malformRequest) => {
			// Arrange:
			const mockServer = createMockServer();
			const route = mockServer.getRoute('/construction/derive').post();

			const request = createValidRequest();
			malformRequest(request);

			// Act:
			mockServer.callRoute(route, { body: request });

			// Assert:
			expect(mockServer.send.calledOnce).to.equal(true);
			expect(mockServer.next.calledOnce).to.equal(true);
			expect(mockServer.res.statusCode).to.equal(500);

			const response = mockServer.send.firstCall.args[0];
			expect(response).to.deep.equal(expectedError);
			expect(() => RosettaError.validateJSON(response)).to.not.throw();
		};

		it('fails when network is unsupported', () => {
			assertRosettaErrorRaised(errors.UNSUPPORTED_NETWORK, request => {
				request.network_identifier.network = 'mainnet';
			});
		});

		it('fails when curve type is unsupported', () => {
			assertRosettaErrorRaised(errors.UNSUPPORTED_CURVE, request => {
				request.public_key.curve_type = 'secp256k1';
			});
		});

		it('fails when public key is invalid', () => {
			assertRosettaErrorRaised(errors.INVALID_PUBLIC_KEY, request => {
				request.public_key.hex_bytes += '0';
			});
		});

		it('returns valid derive response on success', () => {
			// Arrange:
			const mockServer = createMockServer();
			const route = mockServer.getRoute('/construction/derive').post();

			const request = createValidRequest();

			// Act:
			mockServer.callRoute(route, { body: request });

			// Assert:
			expect(mockServer.send.calledOnce).to.equal(true);
			expect(mockServer.next.calledOnce).to.equal(true);
			expect(mockServer.res.statusCode).to.equal(undefined);

			const expectedResponse = new ConstructionDeriveResponse();
			expectedResponse.account_identifier = new AccountIdentifier('TCHEST3QRQS4JZGOO64TH7NFJ2A63YA7TM2K5EQ');

			const response = mockServer.send.firstCall.args[0];
			expect(response).to.deep.equal(expectedResponse);
			expect(() => ConstructionDeriveResponse.validateJSON(response)).to.not.throw();
		});
	});
});
