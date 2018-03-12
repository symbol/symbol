const { expect } = require('chai');
const catapultConnection = require('../../src/connection/catapultConnection');

describe('catapult connection', () => {
	const createTestContext = () => {
		const context = {
			onCalls: {},
			onceCalls: {},
			writeCalls: [],
			removeCalls: {},
			mockConnection: {
				on: (name, handler) => {
					context.onCalls[name] = handler;
					return context.mockConnection;
				},
				once: (name, handler) => { context.onceCalls[name] = handler; },
				write: (payload, callback) => { context.writeCalls.push({ payload, callback }); },
				removeListener: (name, handler) => { context.removeCalls[name] = handler; },

				emit: name => context.onCalls[name]()
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
				.then(() => { throw new Error('promise resolved'); })
				.catch(err => {
					// Assert:
					expect(err.statusCode).to.equal(503);
					expect(err.message).to.equal('connection failed');
				});
		});
	});
});
