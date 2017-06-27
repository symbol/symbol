import catapult from 'catapult-sdk';
import multisigRoutes from '../../../src/plugins/routes/multisigRoutes';
import test from '../../routes/utils/routeTestUtils';

const convert = catapult.utils.convert;

describe('multisig routes', () => {
	const factory = {
		createRouteInfo: (routeName, dbApiName) => ({
			routes: multisigRoutes,
			routeName,
			createDb: (queriedIdentifiers, multisigEntry) => ({
				[dbApiName]: id => {
					queriedIdentifiers.push(id);
					return Promise.resolve(multisigEntry);
				}
			})
		})
	};

	describe('get by account', () => {
		const multisigRouteInfo = factory.createRouteInfo('/account/key/:publicKey/multisig', 'multisigByAccount');
		const Valid_Public_Key = '75D8BB873DA8F5CCA741435DE76A46AFC2840803DEED80E931195B048D77F88C';

		it('returns multisig entry if found', () =>
			// Assert:
			test.route.document.assertReturnsEntityIfFound(multisigRouteInfo, {
				params: { publicKey: Valid_Public_Key },
				paramsIdentifier: convert.hexToUint8(Valid_Public_Key),
				dbEntity: { id: 8 },
				type: 'multisigEntry'
			}));

		it('returns 404 if multisig entry is not found', () =>
			// Assert:
			test.route.document.assertReturnsErrorIfNotFound(multisigRouteInfo, {
				params: { publicKey: Valid_Public_Key },
				paramsIdentifier: convert.hexToUint8(Valid_Public_Key),
				printableParamsIdentifier: Valid_Public_Key,
				dbEntity: undefined
			}));

		it('returns 409 if public key is invalid', () =>
			// Assert:
			test.route.document.assertReturnsErrorForInvalidParams(multisigRouteInfo, {
				params: { publicKey: '12345' }, // odd number of chars
				error: 'publicKey has an invalid format: hex string has unexpected size \'5\''
			}));
	});
});
