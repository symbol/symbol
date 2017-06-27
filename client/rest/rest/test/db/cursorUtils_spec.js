import { expect } from 'chai';
import MongoDb from 'mongodb';
import cursorUtils from '../../src/db/cursorUtils';
import testutils from '../testUtils';
import testDbOptions from './utils/testDbOptions';

describe('cursor utils', () => {
	const test = Object.assign({ constants: { delay: 50 } }, testutils);

	const openDbs = [];
	function connectToDatabase(name) {
		return MongoDb.MongoClient.connect(`${testDbOptions.url}${name}`)
			.then(db => {
				openDbs.push(db);
				return db;
			});
	}

	afterEach(() => {
		// close all db connections
		const promises = [];
		while (0 < openDbs.length)
			promises.push(openDbs.pop().close());

		return Promise.all(promises);
	});

	// connects to local and test databases and returns three collections (test.foo, test.foo2, local.oplog.rs)
	function connectToDatabases() {
		// connect to the local db 3 times because there appears to be a shutdown race otherwise
		// (in MongoDb, a query job is queued after a kill cursor job and complains (correctly) that the cursor can't be found)
		const dbs = {};
		const promise = Promise.all([
			connectToDatabase('test').then(db => { dbs.test = db; }),
			connectToDatabase('local').then(db => { dbs.local = db; }),
			connectToDatabase('local').then(db => { dbs.local2 = db; }),
			connectToDatabase('local').then(db => { dbs.local3 = db; })
		]);

		return promise.then(() => ({
			foo: dbs.test.collection('foo'),
			foo2: dbs.test.collection('foo2'),
			foo3: dbs.test.collection('foo3'),
			oplog: dbs.local.collection('oplog.rs'),
			oplog2: dbs.local2.collection('oplog.rs'),
			oplog3: dbs.local3.collection('oplog.rs')
		}));
	}

	describe('get last document timestamp', () => {
		function getCurrentTimestamp() {
			return new MongoDb.Timestamp(0, (Date.now() / 1000) | 0);
		}

		it('returns current time when collection is empty', () => {
			// Arrange: connect to the test database
			const times = {};
			return connectToDatabase('test')
				.then(db => {
					// Act: get the last document timestamp for an empty collection
					times.start = getCurrentTimestamp().high_;
					return cursorUtils.getLastDocumentTimestamp(db.collection('imaginary'));
				})
				.then(ts => {
					times.stop = getCurrentTimestamp().high_;

					// Assert: the timestamp should be between the start and stop times
					expect(ts.high_).to.not.be.below(times.start);
					expect(ts.high_).to.not.be.above(times.stop);

					// - the ordinal that uniquely indentifies timestamps within the same second is zero
					expect(ts.low_).to.equal(0);
				});
		});

		function insertOneAndGetTimestamp(collection, doc) {
			return collection.insertOne(doc)
				.then(writeResult => {
					const id = writeResult.insertedId;
					const ts = writeResult.result.opTime.ts;
					test.log(`inserted ${id} at ${ts}`);
					return ts;
				});
		}

		it('returns last timestamp when collection is not empty', () =>
			// Arrange:
			connectToDatabases()
				.then(collections =>
					// - insert a document into the foo collection and get its timestamp
					insertOneAndGetTimestamp(collections.foo, { foo: 7 })
						.then(docTs =>
							// Act: get the last timestamp from the oplog
							cursorUtils.getLastDocumentTimestamp(collections.oplog)
								.then(oplogTs => {
									// Assert: the oplog and doc timestamps should match
									expect(oplogTs).to.deep.equal(docTs);
								}))));
	});

	describe('make tailable', () => {
		function getTailingCursor(oplog) {
			// Arrange: get the last document timestamp
			return cursorUtils.getLastDocumentTimestamp(oplog)
				.then(oplogTs => {
					// Act: create a new tailable cursor
					const cursor = oplog.find({ ns: 'test.foo', ts: { $gt: oplogTs } });
					cursorUtils.makeTailable(cursor);
					return cursor;
				});
		}

		function assertFooInserted(doc, value) {
			// Assert:
			expect(doc).to.not.equal(null);
			expect(doc.ns).to.equal('test.foo');
			expect(doc.op).to.equal('i');
			expect(doc.o.foo).to.equal(value);
		}

		it('can retrieve last document inserted before cursor next', () =>
			// Arrange:
			connectToDatabases()
				.then(collections =>
					// Act: get a tailing cursor
					getTailingCursor(collections.oplog)
						.then(cursor => {
							// - insert a document into the foo collection
							const value = Math.random();
							return collections.foo.insertOne({ foo: value })
								// - get the next item from the cursor (one should be ready)
								.then(() => cursor.next())
								.then(doc => {
									// Assert: the cursor returned the last document
									assertFooInserted(doc, value);
								});
						})));

		it('can retrieve last document inserted after cursor next', () =>
			// Arrange:
			connectToDatabases()
				.then(collections =>
					// Act: get a tailing cursor
					getTailingCursor(collections.oplog)
						.then(cursor => {
							// - insert a document into the foo collection after a small delay so that cursor.next() is called first
							const value = Math.random();
							const insertPromise = new Promise(resolve => {
								setTimeout(() => {
									collections.foo.insertOne({ foo: value }).then(() => resolve());
								}, test.constants.delay);
							});

							// - get the next item from the cursor (none should be ready)
							const nextPromise = cursor.next()
								.then(doc => {
									// Assert: the cursor returned the last document
									assertFooInserted(doc, value);
								});

							return Promise.all([insertPromise, nextPromise]);
						})));
	});

	describe('tail data emitter', () => {
		const emitters = [];

		function startEmitter(collection, query) {
			const emitter = cursorUtils.createTailDataEmitter(collection, query);
			emitters.push(emitter);

			return emitter.start().then(() => emitter);
		}

		function assertInsertedDoc(doc, ns, tag, value) {
			// Assert:
			expect(doc.ns).to.equal(ns);
			expect(doc.op).to.equal('i');
			expect(doc.o.tag).to.equal(tag);
			expect(doc.o.value).to.equal(value);
		}

		afterEach(() => {
			// stop all emitters used during the previous test
			while (0 < emitters.length)
				emitters.pop().stop();
		});

		describe('basic events', () => {
			function assertEmittedOpEvent(originalDoc, opFilter, modifyInserted, assertOpDoc) {
				// Arrange:
				return connectToDatabases()
					.then(collections => {
						let docId;

						// Act: wrap an emitter around the oplog collection
						return startEmitter(collections.oplog, { op: opFilter })
							.then(emitter => {
								// - perform a db operation and save the new doc id
								const dbPromise = collections.foo.insertOne(originalDoc)
									.then(writeResult => { docId = writeResult.insertedId; })
									.then(modifyInserted(collections.foo));

								const eventPromise = new Promise(resolve => {
									emitter.once('op', doc => {
										// - wait for the db promise before asserting in order to ensure that docId is set
										dbPromise.then(() => {
											// Assert: the correct event was emitted
											assertOpDoc(docId, doc);
											resolve();
										});
									});
								});

								return Promise.all([eventPromise, dbPromise]);
							});
					});
			}

			it('can emit op event for insert', () => {
				// Assert:
				const value = Math.random();
				return assertEmittedOpEvent(
					{ foo: value, bar: 9 },
					'i',
					() => {}, // insert only
					(docId, doc) => {
						// Assert: the insert event was emitted
						expect(doc.ns).to.equal('test.foo');
						expect(doc.op).to.equal('i');
						expect(doc.o).to.deep.equal({ _id: docId, foo: value, bar: 9 });
					});
			});

			it('can emit op event for update', () => {
				// Assert:
				const value = Math.random();
				return assertEmittedOpEvent(
					{ foo: value, bar: 9 },
					'u',
					collection => collection.update({ foo: value, bar: 9 }, { $set: { bar: 3 } }),
					(docId, doc) => {
						// Assert: the update event was emitted
						expect(doc.ns).to.equal('test.foo');
						expect(doc.op).to.equal('u');
						expect(doc.o).to.deep.equal({ $set: { bar: 3 } });
						expect(doc.o2).to.deep.equal({ _id: docId });
					});
			});

			it('can emit op event for delete', () => {
				// Assert:
				const value = Math.random();
				return assertEmittedOpEvent(
					{ foo: value, bar: 9 },
					'd',
					collection => collection.deleteOne({ foo: value, bar: 9 }),
					(docId, doc) => {
						// Assert: the delete event was emitted
						expect(doc.ns).to.equal('test.foo');
						expect(doc.op).to.equal('d');
						expect(doc.o).to.deep.equal({ _id: docId });
					});
			});
		});

		describe('advanced filtering events', () => {
			it('can emit events from multiple namespaces', () => {
				// Arrange:
				const value = Math.random();
				return connectToDatabases()
					.then(collections => {
						const values = {};
						return startEmitter(collections.oplog, { op: 'i' })
							.then(emitter => {
								// - set up a promise that resolves when the emitter outputs two events
								const eventPromise = new Promise(resolve => {
									emitter.on('op', doc => {
										// - save the ns and value
										values[doc.ns] = doc.o.value;
										if (3 !== Object.keys(values).length)
											return;

										// Assert: two events were emitted from two separate namespaces
										expect(values).to.deep.equal({
											'test.foo': value,
											'test.foo2': value,
											'test.foo3': value
										});
										resolve();
									});
								});

								// Act: insert an item into three collections (foo, foo2, foo3)
								return Promise.all([
									eventPromise,
									collections.foo3.insertOne({ tag: 3, value }),
									collections.foo2.insertOne({ tag: 2, value }),
									collections.foo.insertOne({ tag: 1, value })
								]);
							});
					});
			});

			function createEventPromise(emitter, ns, tag, value) {
				return new Promise(resolve => {
					emitter.once('op', doc => {
						// Assert: the correct event was emitted
						assertInsertedDoc(doc, ns, tag, value);
						resolve();
					});
				});
			}

			it('can only emit events from matching namespace', () => {
				// Arrange:
				const value = Math.random();
				return connectToDatabases()
					.then(collections =>
						// - configure the emitter to only emit events from 'test.foo'
						startEmitter(collections.oplog, { ns: 'test.foo' })
							.then(emitter =>
								// Act:
								Promise.all([
									// - create an event assertion promise (the emitter should emit the object corresponding to its namespace)
									createEventPromise(emitter, 'test.foo', 1, value),
									// - insert an item into three collections (foo, foo2, foo3)
									collections.foo3.insertOne({ tag: 3, value }),
									collections.foo2.insertOne({ tag: 2, value }),
									collections.foo.insertOne({ tag: 1, value })
								])));
			});

			it('can create multiple emitters', () => {
				// Arrange:
				const value = Math.random();
				return connectToDatabases()
					.then(collections => {
						// - create three emitters that subscribe to different namespaces
						const emitterPromise1 = startEmitter(collections.oplog, { ns: 'test.foo' });
						const emitterPromise2 = startEmitter(collections.oplog2, { ns: 'test.foo2' });
						const emitterPromise3 = startEmitter(collections.oplog3, { ns: 'test.foo3' });

						// Act:
						return Promise.all([emitterPromise1, emitterPromise2, emitterPromise3])
							.then(([emitter1, emitter2, emitter3]) => Promise.all([
								// - create event assertion promises (each emitter should emit the object corresponding to its namespace)
								createEventPromise(emitter1, 'test.foo', 1, value),
								createEventPromise(emitter2, 'test.foo2', 2, value),
								createEventPromise(emitter3, 'test.foo3', 3, value),
								// - insert an item into three collections (foo, foo2, foo3)
								collections.foo3.insertOne({ tag: 3, value }),
								collections.foo2.insertOne({ tag: 2, value }),
								collections.foo.insertOne({ tag: 1, value })
							]));
					});
			});
		});

		describe('error handling', () => {
			function createExpectedErrorPromise(emitter, message) {
				return new Promise(resolve => {
					emitter.once('error', err => {
						// Assert: the fired error has the expected message
						expect(err.message).to.equal(message);
						resolve();
					});
				});
			}

			it('emits start errors', () =>
				// Arrange:
				connectToDatabases()
					.then(collections => {
						const emitter = cursorUtils.createTailDataEmitter(collections.oplog);
						emitters.push(emitter);

						// Assert: set up an error handler that resolves when a matching error is raised
						const errorPromise = createExpectedErrorPromise(emitter, 'set stream failed');

						// - queue an error during start
						//   note that event handlers are synchronous, so this really causes a failure during start
						emitter.once('stream', () => {
							throw new Error('set stream failed');
						});

						// Act: start the emitter
						emitter.start();
						return errorPromise;
					}));

			function assertStreamingDataErrorHandling(error, expectedErrorMessage) {
				// Arrange:
				return connectToDatabases()
					.then(collections => {
						const emitter = cursorUtils.createTailDataEmitter(collections.oplog);
						emitters.push(emitter);

						// Assert: set up an error handler that resolves when a matching error is raised
						const errorPromise = createExpectedErrorPromise(emitter, expectedErrorMessage);

						// Act: start the emitter and configure the stream to emit an error after the emitter is started
						emitter.start()
							.then(() => emitter.stream.emit('error', error));

						return errorPromise;
					});
			}

			it('emits stream data errors (Error non-timeout)', () =>
				// Assert:
				assertStreamingDataErrorHandling(new Error('streaming data failed'), 'streaming data failed'));

			it('emits stream data errors (Error timeout)', () =>
				// Assert:
				assertStreamingDataErrorHandling(new Error('timed out'), 'timed out'));

			it('emits stream data errors (MongoError non-timeout)', () =>
				// Assert:
				assertStreamingDataErrorHandling(new MongoDb.MongoError('streaming data failed'), 'streaming data failed'));
		});

		describe('execution', () => {
			it('can stop events', () => {
				// Arrange:
				const value = Math.random();
				return connectToDatabases()
					.then(collections => {
						let numOpEvents = 0;
						return startEmitter(collections.oplog, { ns: 'test.foo' })
							.then(emitter => {
								// - set up a promise
								const eventPromise = new Promise(resolve => {
									emitter.on('op', doc => {
										if (0 !== numOpEvents++)
											return;

										// Assert: the doc corresponds to the first inserted item
										assertInsertedDoc(doc, 'test.foo', 1, value);

										// Act: stop the emitter and insert another item
										emitter.stop();
										collections.foo.insertOne({ tag: 2, value: 2 * value });

										setTimeout(() => {
											// Assert: only one event was raised even though two items were inserted
											expect(numOpEvents).to.equal(1);
											resolve();
										}, test.constants.delay);
									});
								});

								// Act: insert a new item
								return Promise.all([eventPromise, collections.foo.insertOne({ tag: 1, value })]);
							});
					});
			});

			function assertNoErrors(emitter) {
				emitter.once('error', err => {
					// Assert: fail the test if an error was raised
					expect(true, err.message).to.equal(false);
				});
			}

			function createEmitterInOrderPromise(emitter, options) {
				return new Promise(resolve => {
					emitter.on('op', doc => {
						// Assert: all items are returned in order
						assertInsertedDoc(doc, 'test.foo', ++options.lastId, options.value);

						if (options.maxId === options.lastId)
							resolve();
					});
				});
			}

			it('restarts at last ts', () => {
				// Arrange:
				const value = Math.random();
				return connectToDatabases()
					.then(collections =>
						startEmitter(collections.oplog, { ns: 'test.foo' })
							.then(emitter => {
								assertNoErrors(emitter);

								// - set up a promise that checks that all items are returned in order
								//   (even ones that were inserted when the emitter was stopped)
								const options = { lastId: 0, maxId: 7, value };
								const emitterPromise = createEmitterInOrderPromise(emitter, options);

								// - insert 3 items into the collection
								const foo = collections.foo;
								const dbPromise = foo.insertOne({ tag: 1, value })
								.then(() => foo.insertOne({ tag: 2, value }))
								.then(() => foo.insertOne({ tag: 3, value }))
								.then(() => {
									// Act: stop the emitter
									emitter.stop();

									// Sanity: at most 3 events were raised
									expect(options.lastId).to.be.not.above(3);
								})
								// Act: add 2 more items
								.then(() => foo.insertOne({ tag: 4, value }))
								.then(() => foo.insertOne({ tag: 5, value }))
								.then(() => {
									// Sanity: at most 3 events were raised
									expect(options.lastId).to.be.not.above(3);

									// Act: restart the emitter
									return emitter.start();
								})
								// Act: add two more items
								.then(() => foo.insertOne({ tag: 6, value }))
								.then(() => foo.insertOne({ tag: 7, value }));

								return Promise.all([dbPromise, emitterPromise]);
							}));
			});

			it('restarts at last ts after timeout', () => {
				// Arrange:
				const value = Math.random();
				return connectToDatabases()
					.then(collections =>
						startEmitter(collections.oplog, { ns: 'test.foo' })
							.then(emitter => {
								assertNoErrors(emitter);

								// - set up a promise that checks that all items are returned in order
								//   (even ones that were inserted when the emitter was errored)
								const options = { lastId: 0, maxId: 5, value };
								const emitterPromise = createEmitterInOrderPromise(emitter, options);

								// - insert 3 items into the collection
								const foo = collections.foo;
								const dbPromise = foo.insertOne({ tag: 1, value })
									.then(() => foo.insertOne({ tag: 2, value }))
									.then(() => foo.insertOne({ tag: 3, value }))
									.then(() => {
										// Act: simulate a stream exception
										emitter.stream.emit('error', new MongoDb.MongoError('timed out'));

										// Sanity: at most 3 events were raised
										expect(options.lastId).to.be.not.above(3);
									})
									// Act: add 2 more items
									.then(() => foo.insertOne({ tag: 4, value }))
									.then(() => foo.insertOne({ tag: 5, value }));

								return Promise.all([dbPromise, emitterPromise]);
							}));
			});
		});
	});
});
