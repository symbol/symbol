/*
 * Copyright (c) 2016-present,
 * Jaguar0625, gimre, BloodyRookie, Tech Bureau, Corp. All rights reserved.
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

const auth = require('../../src/auth/auth');
const test = require('./utils/authUtils');
const VerifyResult = require('../../src/auth/VerifyResult');
const { expect } = require('chai');

describe('create auth promise', () => {
	const generateServerChallengeRequest = () => {
		const parsedRequest = test.packets.generateServerChallengeRequest();
		const size = 0x00000048;
		const buffer = Buffer.alloc(size);
		buffer.writeInt32LE(size, 0);
		buffer.writeInt32LE(parsedRequest.type, 4);
		parsedRequest.challenge.copy(buffer, 8);
		return buffer;
	};

	const generateClientChallengeResponse = (serverResponse, serverKeyPair) => {
		const parsedResponse = test.packets.generateClientChallengeResponse(serverResponse, serverKeyPair);
		const size = 0x00000048;
		const buffer = Buffer.alloc(size);
		buffer.writeInt32LE(size, 0);
		buffer.writeInt32LE(parsedResponse.type, 4);
		Buffer.from(parsedResponse.signature).copy(buffer, 8);
		return buffer;
	};

	const createPromiseContext = logger => {
		// Arrange:
		const state = {
			writtenPayloads: [],
			isDestroyed: false,
			destroyError: undefined
		};

		const socketEventHandlers = {};
		const serverSocket = {
			on: (eventName, handler) => {
				socketEventHandlers[eventName] = handler;
				return serverSocket;
			},
			write: payload => state.writtenPayloads.push(payload),
			destroy: error => {
				state.isDestroyed = true;
				state.destroyError = error;
				socketEventHandlers.close();
			}
		};

		const clientKeyPair = test.random.keyPair();
		const serverKeyPair = test.random.keyPair();
		const promise = auth.createAuthPromise(serverSocket, clientKeyPair, serverKeyPair.publicKey, logger);

		return {
			state,
			promise,

			authenticate: () => {
				// - create and process the server request
				const serverRequest = generateServerChallengeRequest();
				socketEventHandlers.data(serverRequest);

				// - extract the response challenge
				expect(state.writtenPayloads.length).to.equal(1);
				const serverResponse = test.packets.parseServerChallengeResponse(state.writtenPayloads[0]);

				// - create and process the client response
				const clientResponse = generateClientChallengeResponse(serverResponse, serverKeyPair);
				socketEventHandlers.data(clientResponse);
			},

			authenticateFail: () => {
				// - create and process the server request twice
				const serverRequest = generateServerChallengeRequest();
				socketEventHandlers.data(serverRequest);
				socketEventHandlers.data(serverRequest);
			},

			closeSocket: () => {
				serverSocket.destroy();
			}
		};
	};

	it('resolves promise successfully when verify succeeds', () => {
		// Arrange:
		const context = createPromiseContext();

		// Act:
		context.authenticate();

		// Assert: the socket is not destroyed
		expect(context.state.isDestroyed).to.equal(false);

		return context.promise
			.then(parser => {
				// - the parser is valid and does not have any active listeners
				expect(parser).to.not.equal(undefined);
				expect(parser.impl.emitter.listenerCount('packet')).to.equal(0);
			});
	});

	it('forwards logs to external logger', () => {
		// Arrange:
		const messages = [];
		const logger = message => { messages.push(message); };
		const context = createPromiseContext(logger);

		// Act:
		context.authenticate();

		// Assert: the socket is not destroyed
		expect(context.state.isDestroyed).to.equal(false);

		return context.promise
			.then(() => {
				// - all messages were forwarded to the logger
				expect(messages).to.deep.equal([
					'received data with size 72',
					'writing response of length: 169',
					'received data with size 72',
					'client challenge verified? true'
				]);
			});
	});

	it('rejects promise when verify fails', () => {
		// Arrange:
		const context = createPromiseContext();

		// Act:
		context.authenticateFail();

		// Assert: the socket is destroyed due to a verify failure
		expect(context.state.isDestroyed).to.equal(true);
		expect(context.state.destroyError.verifyResult).to.equal(VerifyResult.malformedData);
		expect(context.state.destroyError.message).to.equal('verify failed with 2');

		return context.promise
			.then(() => { throw Error('promise was not thrown'); })
			.catch(err => {
				// Assert: verify failed due to malformed data
				expect(err.verifyResult).to.equal(VerifyResult.malformedData);
				expect(err.message).to.equal('verify failed with 2');
			});
	});

	it('rejects promise when server socket closes', () => {
		// Arrange:
		const context = createPromiseContext();

		// Act:
		context.closeSocket();

		// Assert: the socket is destroyed externally
		expect(context.state.isDestroyed).to.equal(true);
		expect(context.state.destroyError).to.equal(undefined);

		return context.promise
			.then(() => { throw Error('promise was not thrown'); })
			.catch(err => {
				// Assert: verify failed due to an io error
				expect(err.verifyResult).to.equal(VerifyResult.ioError);
				expect(err.message).to.equal('verify failed with 1');
			});
	});
});
