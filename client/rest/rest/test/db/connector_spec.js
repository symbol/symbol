import { expect } from 'chai';
import EventEmitter from 'events';
import connector from '../../src/db/connector';
import test from '../testUtils';
import testDbOptions from './utils/testDbOptions';

describe('connector', () => {
	const connections = [];

	afterEach(() => {
		// stop all emitters and close database connections used during the previous test
		while (0 < connections.length) {
			const connection = connections.pop();
			connection.emitter.stop();
			connection.db.close();
		}
	});

	it('can connect to database', () =>
		// Act:
		connector.connectToDatabase(testDbOptions.url, 'tokyo')
			.then(db => {
				// Assert:
				expect(db).to.not.equal(null);
				expect(db.s.databaseName).to.equal('tokyo');
			}));

	it('can connect to oplog', () =>
		// Act:
		connector.startTailingOplog(testDbOptions.url)
			.then(connection => {
				connections.push(connection);
				const emitter = connection.emitter;

				// Assert:
				expect(emitter).to.not.equal(null);
				expect(emitter).to.be.instanceof(EventEmitter);
			}));

	it('can connect to oplog and capture op events', () => {
		// Act: create a filtered emitter
		const value = Math.random();
		return connector.startTailingOplog(testDbOptions.url, { ns: 'test.foo', op: 'i' })
			.then(connection => {
				connections.push(connection);
				const emitter = connection.emitter;

				// - create a promise that is resolved when the emitter raises an event
				const emitterPromise = new Promise(resolve => {
					emitter.on('op', data => {
						test.log('received data event');

						// Assert:
						expect(data.o.value).to.equal(value);
						resolve();
					});
				});

				// - insert an item into the test database
				const dbPromise = connector.connectToDatabase(testDbOptions.url, 'test')
					.then(db => db.collection('foo').insertOne({ value }));

				return Promise.all([emitterPromise, dbPromise]);
			});
	});
});
