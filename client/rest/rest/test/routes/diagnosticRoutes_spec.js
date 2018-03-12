const { expect } = require('chai');
const diagnosticRoutes = require('../../src/routes/diagnosticRoutes');
const test = require('./utils/routeTestUtils');

describe('diagnostic routes', () => {
	const executeRoute = (routeName, db, assertResponse) =>
		test.route.executeSingle(diagnosticRoutes.register, routeName, 'get', {}, db, undefined, assertResponse);

	describe('storage', () => {
		const createMockStorageInfoDb = (numBlocks, numTransactions, numAccounts) => ({
			storageInfo: () => Promise.resolve({ numBlocks, numTransactions, numAccounts })
		});

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
		const builder = test.route.document.prepareGetDocumentsRouteTests(diagnosticRoutes.register, {
			route: '/diagnostic/blocks/:height/limit/:limit',
			dbApiName: 'blocksFrom',
			type: 'blockHeaderWithMetadata'
		});

		builder.addValidInputTest({ object: { height: '1234', limit: '4321' }, parsed: [1234, 4321] });
		builder.addEmptyArrayTest({ object: { height: '1234', limit: '4321' }, parsed: [1234, 4321] });

		// notice that this expands to four tests { 'height', 'limit'} x { '10A', '-4321' }
		['height', 'limit'].forEach(property => ['10A', '-4321'].forEach(value => {
			const object = Object.assign({ height: '1234', limit: '4321' }, { [property]: value });
			const errorMessage = `${property} has an invalid format`;
			builder.addInvalidKeyTest({ object, error: errorMessage }, `(${property} with value ${value})`);
		}));
	});
});
