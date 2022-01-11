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

const catapultConnection = require('../../src/connection/catapultConnection');
const { expect } = require('chai');

describe('catapult connection', () => {
	const createTestContext = () => {
		let isEnded = false;
		const context = {
			isEnded: () => isEnded,
			onCalls: {},
			onceCalls: {},
			writeCalls: [],
			removeCalls: {},
			mockConnection: {
				on: (name, handler) => {
					context.onCalls[name] = handler;
					return context.mockConnection;
				},
				once: (name, handler) => {
					context.onceCalls[name] = handler;
				},
				write: (payload, callback) => {
					context.writeCalls.push({ payload, callback });
				},
				removeListener: (name, handler) => {
					context.removeCalls[name] = handler;
				},

				emit: name => context.onCalls[name](),

				end: () => {
					isEnded = true;
				}
			}
		};
		return context;
	};

	describe('send', () => {
		const assertSend = (testName, assertCallback) => {
			it(testName, () => {
				// Arrange:
				const payload = { test: 12345 };
				const context = createTestContext();

				// Act:
				const promise = catapultConnection.wrap(context.mockConnection).send(payload);

				// Assert:
				expect(context.onCalls).to.deep.equal({});
				expect(context.onceCalls).to.have.all.keys('close');
				expect(context.writeCalls.length).to.equal(1);
				expect(context.writeCalls[0].payload).to.deep.equal(payload);
				expect(context.removeCalls).to.deep.equal({});

				return assertCallback(context, promise);
			});
		};

		assertSend('sending the data resolves the promise', (context, promise) => {
			// Act:
			context.writeCalls[0].callback();

			return promise.then(() => {
				// Assert:
				expect(context.removeCalls).to.have.all.keys('close');
				expect(context.removeCalls.close).to.equal(context.onceCalls.close);
			});
		});

		assertSend('closing connection rejects the promise', (context, promise) => {
			// Act:
			context.onceCalls.close();

			return promise
				.then(() => {
					throw new Error('promise resolved');
				})
				.catch(err => {
					// Assert:
					expect(err.statusCode).to.equal(503);
					expect(err.message).to.equal('connection failed');
				});
		});
	});

	describe('pushPull', () => {
		const assertPushPull = (testName, assertCallback) => {
			it(testName, () => {
				// Arrange:
				const payload = { test: 12345 };
				const context = createTestContext();

				// Act:
				const promise = catapultConnection.wrap(context.mockConnection).pushPull(payload, 1000);

				// Assert:
				expect(context.onCalls).to.deep.equal({});
				expect(context.onceCalls).to.have.all.keys('close');
				expect(context.writeCalls.length).to.equal(1);
				expect(context.writeCalls[0].payload).to.deep.equal(payload);
				expect(context.removeCalls).to.deep.equal({});

				return assertCallback(context, promise);
			});
		};

		assertPushPull('sending payload returns data', (context, promise) => {
			// Act:
			context.writeCalls[0].callback();

			setTimeout(() => {
				context.onCalls.data(Buffer.from([24, 0, 0, 0, 188, 2, 0, 0]));
				context.onCalls.data(Buffer.from([80, 71, 26, 165, 16, 0, 0, 0, 80, 71, 26, 165, 16, 0, 0, 0]));
			}, 10);

			return promise.then(packet => {
				// Assert:
				expect(packet).to.be.deep.equal({
					type: 700,
					size: 24,
					payload: Buffer.from([80, 71, 26, 165, 16, 0, 0, 0, 80, 71, 26, 165, 16, 0, 0, 0])
				});
				expect(context.removeCalls).to.have.all.keys('close');
				expect(context.removeCalls.close).to.equal(context.onceCalls.close);
				expect(context.isEnded()).to.be.equal(true);
			});
		});

		assertPushPull('closing connection rejects the promise', (context, promise) => {
			// Act:
			context.onceCalls.close();

			return promise
				.then(() => {
					throw new Error('promise resolved');
				})
				.catch(err => {
					// Assert:
					expect(err.statusCode).to.equal(503);
					expect(err.message).to.equal('connection failed');
				});
		});

		assertPushPull('not receiving the data before timeout rejects the promise', (context, promise) => {
			// Act:
			context.writeCalls[0].callback();

			return promise
				.then(() => {
					throw new Error('promise resolved');
				})
				.catch(err => {
					// Assert:
					expect(err.statusCode).to.equal(503);
					expect(err.message).to.equal('connection failed');
					expect(context.isEnded()).to.be.equal(true);
				});
		});

		assertPushPull(
			'closing connection after receiving the first part of the packet rejects the promise',
			(context, promise) => {
				// Act:
				context.writeCalls[0].callback();

				setTimeout(() => {
					context.onCalls.data(Buffer.from([24, 0, 0, 0, 188, 2, 0, 0]));
					context.onceCalls.close();
				}, 10);

				return promise
					.then(() => {
						throw new Error('promise resolved');
					})
					.catch(err => {
						// Assert:
						expect(err.statusCode).to.equal(503);
						expect(err.message).to.equal('connection failed');
					});
			}
		);
	});
});
