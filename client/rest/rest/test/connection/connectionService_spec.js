const { expect } = require('chai');
const catapult = require('catapult-sdk');
const { createConnectionService } = require('../../src/connection/connectionService');

const { convert } = catapult.utils;
const { createKeyPairFromPrivateKeyString } = catapult.crypto;

describe('connection service', () => {
	const Test_Config = {
		apiNode: {
			host: 'test hostname',
			port: 12345,
			publicKey: '11223344556677889900AABBCCDDEEFF'
		},
		clientPrivateKey: '0000000000000000000000000000000000000000000000000000000000000000'
	};
	Test_Config.expected = {
		clientKeyPair: createKeyPairFromPrivateKeyString(Test_Config.clientPrivateKey),
		apiNodePublicKey: convert.hexToUint8(Test_Config.apiNode.publicKey)
	};

	const createTestContext = () => {
		const routeContext = {
			onCalls: {},
			mockSockets: [], // stores all mock sockets created by factory
			lastSocket: () => routeContext.mockSockets[routeContext.mockSockets.length - 1],
			numConnectionFactoryCalls: 0,
			mockConnectionFactory: (port, host) => {
				++routeContext.numConnectionFactoryCalls;
				routeContext.host = host;
				routeContext.port = port;

				const mockSocket = {
					numWrites: 0,
					on: (eventName, eventHandler) => {
						routeContext.onCalls[eventName] = eventHandler;
						return mockSocket;
					},
					once: () => {},
					write: () => {
						++mockSocket.numWrites;
					}
				};

				routeContext.mockSockets.push(mockSocket);
				return routeContext.lastSocket();
			},
			auth: {},
			createAuthPromise: promiseFactory =>
				(serverSocket, clientKeyPair, apiNodePublicKey) => {
					routeContext.auth.serverSocket = serverSocket;
					routeContext.auth.clientKeyPair = clientKeyPair;
					routeContext.auth.apiNodePublicKey = apiNodePublicKey;
					return promiseFactory();
				},
			createSuccessAuthPromise: () => routeContext.createAuthPromise(() => Promise.resolve()),
			createFailureAuthPromise: () => routeContext.createAuthPromise(() => Promise.reject(Error('failure auth promise')))
		};
		return routeContext;
	};

	const assertData = context => {
		expect(context.host).to.equal(Test_Config.apiNode.host);
		expect(context.port).to.equal(Test_Config.apiNode.port);
		expect(context.numConnectionFactoryCalls).to.equal(1);

		// note: this is not deep equal on purpose
		expect(context.auth.serverSocket).to.equal(context.lastSocket());
		expect(context.auth.clientKeyPair).to.deep.equal(Test_Config.expected.clientKeyPair);
		expect(context.auth.apiNodePublicKey).to.deep.equal(Test_Config.expected.apiNodePublicKey);

		expect(context.onCalls).to.have.all.keys(['close', 'error']);
	};

	it('authentication success is forwarded', () => {
		// Arrange:
		const context = createTestContext();
		const connectionService = createConnectionService(Test_Config, context.mockConnectionFactory, context.createSuccessAuthPromise());

		// Act:
		return connectionService.lease().then(connection => {
			// Assert:
			assertData(context);

			// - the connection sends to the last socket
			connection.send();
			expect(context.lastSocket().numWrites).to.equal(1);
		});
	});

	it('authentication failure is forwarded', () => {
		// Arrange:
		const context = createTestContext();
		const connectionService = createConnectionService(Test_Config, context.mockConnectionFactory, context.createFailureAuthPromise());

		// Act:
		const promise = connectionService.lease();
		return promise.catch(err => {
			// Assert:
			assertData(context);
			expect(err.message).to.equal('failure auth promise');
		});
	});

	it('connection failure is forwarded', () => {
		// Arrange:
		const context = createTestContext();
		const promiseInterruptedByDisconnect = context.createAuthPromise(() => {
			// note: assumption is that connection() registers 'close' event and
			// that it rejects promise if close occurred
			context.onCalls.close();
			return Promise.resolve();
		});
		const connectionService = createConnectionService(Test_Config, context.mockConnectionFactory, promiseInterruptedByDisconnect);

		// Act:
		return connectionService.lease()
			.then(() => { throw new Error('promise resolved'); })
			.catch(err => {
				// Assert:
				assertData(context);
				expect(err.statusCode).to.equal(503);
				expect(err.message).to.equal('connection failed');
			});
	});

	const assertAfterFirstConnectionSucceeded = (name, followingActions) => {
		it(`first connection succeeded, ${name}`, () => {
			// Arrange:
			const context = createTestContext();
			const connectionService = createConnectionService(
				Test_Config,
				context.mockConnectionFactory,
				context.createSuccessAuthPromise()
			);

			// Act:
			return connectionService.lease().then(firstConnection => {
				assertData(context);

				return followingActions(context, connectionService, firstConnection);
			});
		});
	};

	assertAfterFirstConnectionSucceeded('connection is cached', (context, connectionService, firstConnection) =>
		// Act: connection succeeded, let's try to make another one, and check that they match:
		connectionService.lease().then(secondConnection => {
			// Assert: connection was created only once
			expect(context.numConnectionFactoryCalls).to.equal(1);
			expect(secondConnection).to.equal(firstConnection);
		}));

	assertAfterFirstConnectionSucceeded('connection will be restored after close', (context, connectionService, firstConnection) => {
		// Act: break the connection
		context.onCalls.close();

		return connectionService.lease().then(secondConnection => {
			// Assert:
			expect(context.numConnectionFactoryCalls).to.equal(2);
			expect(secondConnection).to.not.equal(firstConnection);

			// - the first connection sends to the first socket
			firstConnection.send();
			expect(context.mockSockets[0].numWrites).to.equal(1);

			// - the second connection sends to the second socket
			secondConnection.send();
			expect(context.mockSockets[1].numWrites).to.equal(1);
		});
	});

	assertAfterFirstConnectionSucceeded('error alone does not affect connection', (context, connectionService, firstConnection) => {
		// Act: emit an error
		context.onCalls.error(new Error());

		return connectionService.lease().then(secondConnection => {
			// Assert: connection was created only once
			expect(context.numConnectionFactoryCalls).to.equal(1);
			expect(secondConnection).to.equal(firstConnection);
		});
	});
});
