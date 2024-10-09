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

import Amount from '../../../src/plugins/rosetta/openApi/model/Amount.js';
import ConstructionDeriveRequest from '../../../src/plugins/rosetta/openApi/model/ConstructionDeriveRequest.js';
import Currency from '../../../src/plugins/rosetta/openApi/model/Currency.js';
import RosettaApiError from '../../../src/plugins/rosetta/openApi/model/Error.js';
import {
	RosettaErrorFactory,
	RosettaPublicKeyProcessor,
	evaluateOperationsAndUpdateAmounts,
	extractTransferDescriptorAt,
	rosettaPostRouteWithNetwork
} from '../../../src/plugins/rosetta/rosettaUtils.js';
import { expect } from 'chai';
import { PublicKey } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';

describe('rosetta utils', () => {
	// region test utils

	const createRosettaTransfer = (index, address, amount) => ({
		operation_identifier: { index },
		type: 'transfer',
		account: { address },
		amount: 'string' === typeof amount ? { value: amount } : amount
	});

	const createRosettaCosignatory = (index, address) => ({
		operation_identifier: { index },
		type: 'cosign',
		account: { address }
	});

	// endregion

	// region RosettaErrorFactory

	describe('RosettaErrorFactory', () => {
		it('all have unique codes', () => {
			// Arrange: filter out properties added to every class
			const defaultClassPropertyNames = Object.getOwnPropertyNames(class C {});
			const errorNames = Object.getOwnPropertyNames(RosettaErrorFactory)
				.filter(name => !defaultClassPropertyNames.includes(name));

			// Act:
			const errorCodeSet = new Set(errorNames.map(name => RosettaErrorFactory[name].apiError.code));

			// Assert:
			expect(errorCodeSet.size).to.equal(errorNames.length);
		});
	});

	// endregion

	// region RosettaPublicKeyProcessor

	describe('RosettaPublicKeyProcessor', () => {
		// use Symbol types for testing
		const createPublicKeyProcessor = () => new RosettaPublicKeyProcessor('desired-curve', Network.TESTNET, PublicKey);
		const createRosettaPublicKey = (hexBytes, curveType) => ({
			hex_bytes: hexBytes,
			curve_type: curveType
		});

		describe('parsePublicKey', () => {
			it('fails when public key curve type is unsupported', () => {
				// Arrange:
				const processor = createPublicKeyProcessor();
				const rosettaPublicKey = createRosettaPublicKey(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
					'other-curve'
				);

				// Act + Assert:
				expect(() => processor.parsePublicKey(rosettaPublicKey)).to.throw(RosettaErrorFactory.UNSUPPORTED_CURVE.message);
			});

			it('fails when public key is invalid', () => {
				// Arrange:
				const processor = createPublicKeyProcessor();
				const rosettaPublicKey = createRosettaPublicKey(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690',
					'desired-curve'
				);

				// Act + Assert:
				expect(() => processor.parsePublicKey(rosettaPublicKey)).to.throw(RosettaErrorFactory.INVALID_PUBLIC_KEY.message);
			});

			it('succeeds when public key is valid', () => {
				// Arrange:
				const processor = createPublicKeyProcessor();
				const rosettaPublicKey = createRosettaPublicKey(
					'93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7',
					'desired-curve'
				);

				// Act + Assert:
				const publicKey = processor.parsePublicKey(rosettaPublicKey);

				// Assert:
				expect(publicKey).to.deep.equal(new PublicKey('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'));
			});
		});

		describe('buildAddressToPublicKeyMap', () => {
			it('fails when any public key curve type is unsupported', () => {
				// Arrange:
				const processor = createPublicKeyProcessor();
				const rosettaPublicKeys = [
					createRosettaPublicKey('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7', 'desired-curve'),
					createRosettaPublicKey('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623', 'other-curve'),
					createRosettaPublicKey('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6', 'desired-curve')
				];

				// Act + Assert:
				expect(() => processor.buildAddressToPublicKeyMap(rosettaPublicKeys))
					.to.throw(RosettaErrorFactory.UNSUPPORTED_CURVE.message);
			});

			it('fails when any public key is invalid', () => {
				// Arrange:
				const processor = createPublicKeyProcessor();
				const rosettaPublicKeys = [
					createRosettaPublicKey('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7', 'desired-curve'),
					createRosettaPublicKey('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E86', 'desired-curve'),
					createRosettaPublicKey('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6', 'desired-curve')
				];

				// Act + Assert:
				expect(() => processor.buildAddressToPublicKeyMap(rosettaPublicKeys))
					.to.throw(RosettaErrorFactory.INVALID_PUBLIC_KEY.message);
			});

			it('succeeds when all public keys are valid', () => {
				// Arrange:
				const processor = createPublicKeyProcessor();
				const rosettaPublicKeys = [
					createRosettaPublicKey('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7', 'desired-curve'),
					createRosettaPublicKey('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623', 'desired-curve'),
					createRosettaPublicKey('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6', 'desired-curve')
				];

				// Act + Assert:
				const addressToPublicKeyMap = processor.buildAddressToPublicKeyMap(rosettaPublicKeys);

				// Assert:
				expect(addressToPublicKeyMap).to.deep.equal({
					TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ:
						new PublicKey('93A62514605D7DE3BDF699C54AE850CA3DACDC8CCA41A69C786CE97FA5F690D7'),
					TBPXHVTQBGRTSYXP4Q55EEUIV73UFC2D72KCWXQ:
						new PublicKey('3119DA3BFF57385BB6F051B8A454F219CE519D28E50D5653F5F457486E9E8623'),
					TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI:
						new PublicKey('ED7FE5166BDC65D065667630B96362B3E57AFCA2B557B57E02022631C8C8F1A6')
				});
			});
		});
	});

	// endregion

	// region rosettaPostRouteWithNetwork

	describe('rosettaPostRouteWithNetwork', () => {
		// use /construction/derive types in these tests

		const createValidRequest = () => ({
			network_identifier: {
				blockchain: 'Foo',
				network: 'testnet'
			},
			public_key: {
				hex_bytes: 'E85D10BF47FFBCE2230F70CB43ED2DDE04FCF342524B383972F86EA1FF773C79',
				curve_type: 'edwards25519'
			}
		});

		const createRosettaRouteTestSetup = () => {
			const routeContext = { numNextCalls: 0 };
			const next = () => { ++routeContext.numNextCalls; };

			routeContext.responses = [];
			routeContext.headers = [];
			const res = {
				statusCode: 200,
				send: response => { routeContext.responses.push(response); },
				setHeader: (name, value) => { routeContext.headers.push({ name, value }); }
			};

			return { routeContext, next, res };
		};

		const assertRosettaErrorRaised = (routeContext, res, expectedError) => {
			expect(routeContext.numNextCalls).to.equal(1);
			expect(routeContext.responses.length).to.equal(1);
			expect(res.statusCode).to.equal(500);

			const response = routeContext.responses[0];
			expect(response).to.deep.equal(expectedError.apiError);
			expect(() => RosettaApiError.validateJSON(response)).to.not.throw();
		};

		it('fails when request is invalid', async () => {
			// Arrange: corrupt the request by removing a required subfield
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();
			delete request.network_identifier.network;

			// Act:
			const postHandler = rosettaPostRouteWithNetwork({ blockchain: 'Foo', network: 'testnet' }, ConstructionDeriveRequest, () => {});
			await postHandler({ body: request }, res, next);

			// Assert:
			assertRosettaErrorRaised(routeContext, res, RosettaErrorFactory.INVALID_REQUEST_DATA);
		});

		const assertFailsWhenIsNotTargetNetwork = async blockchainDescriptor => {
			// Arrange:
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();

			// Act:
			const postHandler = rosettaPostRouteWithNetwork(blockchainDescriptor, ConstructionDeriveRequest, () => {});
			await postHandler({ body: request }, res, next);

			// Assert:
			assertRosettaErrorRaised(routeContext, res, RosettaErrorFactory.UNSUPPORTED_NETWORK);
		};

		it('fails when blockchain is invalid', () => assertFailsWhenIsNotTargetNetwork({ blockchain: 'Symbol', network: 'testnet' }));

		it('fails when network is invalid', () => assertFailsWhenIsNotTargetNetwork({ blockchain: 'Foo', network: 'mainnet' }));

		const assertFailsWhenHandlerRaisesError = async (raisedError, expectedError) => {
			// Arrange:
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();

			// Act:
			const postHandler = rosettaPostRouteWithNetwork(
				{ blockchain: 'Foo', network: 'testnet' },
				ConstructionDeriveRequest, () => Promise.reject(raisedError)
			);
			await postHandler({ body: request }, res, next);

			// Assert:
			assertRosettaErrorRaised(routeContext, res, expectedError);
		};

		it('fails when handler raises error (rosetta)', () => assertFailsWhenHandlerRaisesError(
			RosettaErrorFactory.INVALID_PUBLIC_KEY,
			RosettaErrorFactory.INVALID_PUBLIC_KEY
		));

		it('fails when handler raises error (unknown)', () => assertFailsWhenHandlerRaisesError(
			Error('unknown error'),
			RosettaErrorFactory.INTERNAL_SERVER_ERROR
		));

		const assertSuccessWhenValid = async handler => {
			// Arrange:
			const { routeContext, next, res } = createRosettaRouteTestSetup();
			const request = createValidRequest();

			// Act:
			const postHandler = rosettaPostRouteWithNetwork({ blockchain: 'Foo', network: 'testnet' }, ConstructionDeriveRequest, handler);
			await postHandler({ body: request }, res, next);

			// Assert:
			expect(routeContext.numNextCalls).to.equal(1);
			expect(routeContext.responses.length).to.equal(1);
			expect(res.statusCode).to.equal(200);
			expect(routeContext.responses[0]).to.deep.equal({ foo: 'testnet' });
		};

		it('succeeds when blockchain descriptor is valid', () => assertSuccessWhenValid(typedRequest => ({
			foo: typedRequest.network_identifier.network
		})));

		it('succeeds when blockchain descriptor is valid (async)', () => assertSuccessWhenValid(typedRequest => (Promise.resolve({
			foo: typedRequest.network_identifier.network
		}))));
	});

	// endregion

	// region extractTransferDescriptorAt

	describe('extractTransferDescriptorAt', () => {
		it('fails when transfer is hanging', () => {
			// Arrange:
			const operations = [
				createRosettaCosignatory(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				createRosettaTransfer(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100')
			];

			// Act + Assert:
			expect(() => extractTransferDescriptorAt(operations, 1)).to.throw(RosettaErrorFactory.INVALID_REQUEST_DATA.message);
		});

		it('fails when transfer is unpaired', () => {
			// Arrange:
			const operations = [
				createRosettaCosignatory(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				createRosettaTransfer(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaCosignatory(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ')
			];

			// Act + Assert:
			expect(() => extractTransferDescriptorAt(operations, 0)).to.throw(RosettaErrorFactory.INVALID_REQUEST_DATA.message);
			expect(() => extractTransferDescriptorAt(operations, 1)).to.throw(RosettaErrorFactory.INVALID_REQUEST_DATA.message);
		});

		it('fails when transfer has mismatched amounts unpaired', () => {
			// Arrange:
			const operations = [
				createRosettaCosignatory(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				createRosettaTransfer(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-99')
			];

			// Act + Assert:
			expect(() => extractTransferDescriptorAt(operations, 1)).to.throw(RosettaErrorFactory.INVALID_REQUEST_DATA.message);
		});

		it('succeeds when transfer is matched (credit first)', () => {
			// Arrange:
			const operations = [
				createRosettaCosignatory(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				createRosettaTransfer(1, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100'),
				createRosettaTransfer(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-100')
			];

			// Act:
			const transferDescriptor = extractTransferDescriptorAt(operations, 1);

			// Assert:
			expect(transferDescriptor).to.deep.equal({
				senderAddress: 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
				recipientAddress: 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
				amount: 100n
			});
		});

		it('succeeds when transfer is matched (debit first)', () => {
			// Arrange:
			const operations = [
				createRosettaCosignatory(0, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI'),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', '-100'),
				createRosettaTransfer(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', '100')
			];

			// Act:
			const transferDescriptor = extractTransferDescriptorAt(operations, 1);

			// Assert:
			expect(transferDescriptor).to.deep.equal({
				senderAddress: 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ',
				recipientAddress: 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI',
				amount: 100n
			});
		});
	});

	// endregion

	// region evaluateOperationsAndUpdateAmounts

	describe('evaluateOperationsAndUpdateAmounts', () => {
		const createRosettaAmount = (amount, currencyName) => new Amount(amount, new Currency(currencyName, 1));

		it('can filter operations by address', () => {
			// Arrange:
			const amounts = [];
			const operations = [
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('100', 'foo.bar')),
				createRosettaTransfer(2, 'TARZARAKDFNYFVFANAIAHCYUADHHZWT2WP2I7GI', createRosettaAmount('-100', 'foo.bar')),
				createRosettaTransfer(3, 'TDI2ZPA7U72GHU2ZDP4C4J6T5YMFSLWEW4OZQKI', createRosettaAmount('80', 'foo.bar')),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('-80', 'foo.bar'))
			];

			// Act:
			evaluateOperationsAndUpdateAmounts('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', amounts, operations);

			// Assert:
			expect(amounts).to.deep.equal([
				createRosettaAmount('20', 'foo.bar')
			]);
		});

		it('can filter operations by type', () => {
			// Arrange:
			const amounts = [];
			const operations = [
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('100', 'foo.bar')),
				createRosettaCosignatory(2, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
				createRosettaCosignatory(3, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ'),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('-80', 'foo.bar'))
			];

			// Act:
			evaluateOperationsAndUpdateAmounts('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', amounts, operations);

			// Assert:
			expect(amounts).to.deep.equal([
				createRosettaAmount('20', 'foo.bar')
			]);
		});

		it('can add multiple new currencies', () => {
			// Arrange:
			const amounts = [];
			const operations = [
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('100', 'foo.bar')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('10', 'cat.dog')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('12', 'alice.huge')),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('-80', 'foo.bar'))
			];

			// Act:
			evaluateOperationsAndUpdateAmounts('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', amounts, operations);

			// Assert:
			expect(amounts).to.deep.equal([
				createRosettaAmount('20', 'foo.bar'),
				createRosettaAmount('10', 'cat.dog'),
				createRosettaAmount('12', 'alice.huge')
			]);
		});

		it('can merge multiple existing currencies', () => {
			// Arrange:
			const amounts = [
				createRosettaAmount('100', 'foo.bar'),
				createRosettaAmount('200', 'cat.dog'),
				createRosettaAmount('300', 'alice.huge')
			];
			const operations = [
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('100', 'foo.bar')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('10', 'cat.dog')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('12', 'alice.huge')),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('-80', 'foo.bar'))
			];

			// Act:
			evaluateOperationsAndUpdateAmounts('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', amounts, operations);

			// Assert:
			expect(amounts).to.deep.equal([
				createRosettaAmount('120', 'foo.bar'),
				createRosettaAmount('210', 'cat.dog'),
				createRosettaAmount('312', 'alice.huge')
			]);
		});

		it('can merge multiple new and existing currencies', () => {
			// Arrange:
			const amounts = [
				createRosettaAmount('100', 'foo.bar'),
				createRosettaAmount('300', 'alice.huge')
			];
			const operations = [
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('100', 'foo.bar')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('10', 'cat.dog')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('12', 'alice.huge')),
				createRosettaTransfer(4, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('-80', 'foo.bar')),
				createRosettaTransfer(1, 'TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', createRosettaAmount('44', 'alice.big'))
			];

			// Act:
			evaluateOperationsAndUpdateAmounts('TCULEHFGXY7E6TWBXH7CVKNKFSUH43RNWW52NWQ', amounts, operations);

			// Assert:
			expect(amounts).to.deep.equal([
				createRosettaAmount('120', 'foo.bar'),
				createRosettaAmount('312', 'alice.huge'),
				createRosettaAmount('10', 'cat.dog'),
				createRosettaAmount('44', 'alice.big')
			]);
		});
	});

	// endregion
});
