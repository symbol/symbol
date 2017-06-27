import { expect } from 'chai';
import diagnosticRoutes from '../../src/routes/diagnosticRoutes';
import test from './utils/routeTestUtils';

describe('diagnostic routes', () => {
	function executeRoute(routeName, db, assertResponse) {
		return test.route.executeSingle(diagnosticRoutes.register, routeName, 'get', {}, db, assertResponse);
	}

	describe('storage', () => {
		function createMockStorageInfoDb(numBlocks, numTransactions, numAccounts) {
			return {
				storageInfo: () => Promise.resolve({ numBlocks, numTransactions, numAccounts })
			};
		}

		it('can retrieve info', () => {
			// Arrange:
			const db = createMockStorageInfoDb(2, 64, 9);

			// Act:
			return executeRoute('/diagnostic/storage', db, response => {
				// Assert:
				expect(response).to.deep.equal({
					payload: { numBlocks: 2, numTransactions: 64, numAccounts: 9 },
					type: 'storageInfo'
				});
			});
		});
	});

	describe('blocks', () => {
		const diagnosticBlocksRouteInfo = {
			routes: diagnosticRoutes,
			routeName: '/diagnostic/blocks/:height/count/:count',
			createDb: (queriedIdentifiers, entity) => ({
				blocksFrom: (height, count) => {
					queriedIdentifiers.push({ height, count });
					return Promise.resolve(entity);
				}
			})
		};

		function createTraitsForDiagnosticBlocks(options) {
			return {
				params: { height: options.height, count: options.count },
				paramsIdentifier: options.expected,
				dbEntity: [1, 2, 3],
				type: 'blockHeaderWithMetadata'
			};
		}

		function assertSuccess(options) {
			return test.route.document.assertReturnsEntityIfFound(diagnosticBlocksRouteInfo, createTraitsForDiagnosticBlocks(options));
		}

		function assertFailure(name, options) {
			return test.route.document.assertReturnsErrorForInvalidParams(diagnosticBlocksRouteInfo, {
				params: { height: options.height, count: options.count },
				error: `${name} has an invalid format: must be non-negative number`
			});
		}

		it('returns blocks if request is valid', () =>
			// Assert:
			assertSuccess({ height: '1234', count: '4321', expected: { height: 1234, count: 4321 } }));

		for (const property of ['height', 'count']) {
			it(`throws an error if ${property} is invalid`, () =>
				Promise.all(['-12345', '50A'].map(invalidValue =>
					// Assert:
					assertFailure(property, Object.assign({ height: '1234', count: '25' }, { [property]: invalidValue })))));
		}
	});
});
