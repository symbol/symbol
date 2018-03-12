const { expect } = require('chai');
const connector = require('../../src/db/connector');
const testDbOptions = require('./utils/testDbOptions');

describe('connector', () => {
	const connections = [];

	afterEach(() => {
		// close database connections used during the previous test
		while (0 < connections.length) {
			const connection = connections.pop();
			connection.close();
		}
	});

	it('can connect to database', () =>
		// Act:
		connector.connectToDatabase(testDbOptions.url, 'tokyo')
			.then(client => {
				connections.push(client);

				// Assert:
				expect(client).to.not.equal(null);
				expect(client.db().s.databaseName).to.equal('tokyo');
			}));
});
