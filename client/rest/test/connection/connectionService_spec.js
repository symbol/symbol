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

const { MockSocket } = require('./MockSocket');
const { createConnectionService } = require('../../src/connection/connectionService');
const { expect } = require('chai');
const sinon = require('sinon');
const tls = require('tls');

describe('connection service', () => {
	const sockets = [];
	let tlsConnectStub;
	const testConfig = {
		apiNode: {
			host: 'test hostname',
			port: 12345,
			key: '',
			certificate: '',
			caCertificate: ''
		}
	};
	let connectionService;

	beforeEach(() => {
		tlsConnectStub = sinon.stub(tls, 'connect').callsFake(() => {
			const fakeSocket = new MockSocket();
			sockets.push(fakeSocket);
			return fakeSocket;
		});
		connectionService = createConnectionService(testConfig);
	});

	afterEach(() => {
		tlsConnectStub.restore();
		tlsConnectStub = undefined;
		sockets.splice(0, sockets.length);
		connectionService = undefined;
	});

	it('authentication success is forwarded', () => {
		// Act:
		const leasePromise = connectionService.lease().then(connection => {
			connection.send();

			// Assert:
			expect(sockets[0].numWrites).to.equal(1);
		});
		sockets[0].authorized = true;
		sockets[0].fireEvent('secureConnect');

		return leasePromise;
	});

	it('connection close is forwarded', () => {
		// Act:
		const leasePromise = connectionService.lease().catch(error => {
			// Assert:
			expect(error.message).to.equal('connection failed');
		});
		sockets[0].authorized = true;
		sockets[0].fireEvent('close');

		return leasePromise;
	});

	describe('connection lease', () => {
		it('connection is cached/reused', () => {
			// Act:
			const leasePromise = connectionService.lease().then(connection1 =>
				connectionService.lease().then(connection2 => {
					// Assert:
					expect(connection2).to.equal(connection1);
				}));

			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return leasePromise;
		});

		it('connection will be recreated after close', () => {
			const leasePromise = connectionService.lease().then(connection1 => {
				sockets[0].fireEvent('close');

				const leasePromise2 = connectionService.lease().then(connection2 => {
					// Assert:
					expect(connection2).to.not.equal(connection1);

					connection2.send();
					expect(sockets[0].numWrites).to.equal(0);
					expect(sockets[1].numWrites).to.equal(1);
				});

				sockets[1].authorized = true;
				sockets[1].fireEvent('secureConnect');

				return leasePromise2;
			});
			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return leasePromise;
		});

		it('error event does not affect connection', () => {
			// Act:
			const leasePromise = connectionService.lease().then(connection => {
				sockets[0].fireEvent('error');
				connection.send();
				expect(sockets[0].numWrites).to.equal(1);
			});
			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return leasePromise;
		});
	});

	describe('connection singleUse', () => {
		it('connection is not cached/reused', () => {
			// Act:
			const singleUsePromise = connectionService.singleUse().then(connection1 => {
				const singleUsePromise2 = connectionService.singleUse().then(connection2 => {
					// Assert:
					expect(connection2).to.not.equal(connection1);
				});
				sockets[1].authorized = true;
				sockets[1].fireEvent('secureConnect');

				return singleUsePromise2;
			});
			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return singleUsePromise;
		});

		it('new connection is created after close', () => {
			const singleUsePromise = connectionService.singleUse().then(connection1 => {
				sockets[0].fireEvent('close');

				const singleUsePromise2 = connectionService.singleUse().then(connection2 => {
					// Assert:
					expect(connection2).to.not.equal(connection1);

					connection2.send();
					expect(sockets[0].numWrites).to.equal(0);
					expect(sockets[1].numWrites).to.equal(1);
				});

				sockets[1].authorized = true;
				sockets[1].fireEvent('secureConnect');

				return singleUsePromise2;
			});
			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return singleUsePromise;
		});

		it('error event does not affect connection', () => {
			// Act:
			const singleUsePromise = connectionService.singleUse().then(connection => {
				sockets[0].fireEvent('error');
				connection.send();
				expect(sockets[0].numWrites).to.equal(1);
			});
			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return singleUsePromise;
		});
	});

	describe('handles first simultaneous connections correctly when authorization is delayed', () => {
		it('on successful authorization both parties should lease the same connection', () => {
			// Act:
			let firstConn;
			let secondConn;
			const leasePromises = Promise.all([
				connectionService.lease().then(firstConnection => {
					firstConn = firstConnection;
				}),
				connectionService.lease().then(secondConnection => {
					secondConn = secondConnection;
				})
			]).then(() => {
				// Assert:
				expect(secondConn).to.equal(firstConn);
			});

			// unblocks the first promise (simulates delayed acceptance of authorization)
			sockets[0].authorized = true;
			sockets[0].fireEvent('secureConnect');

			return leasePromises;
		});

		it('on failed authorization both parties should fail on the same pending authorization promise', () => {
			// Act:
			const leasePromises = Promise.all([
				connectionService.lease(),
				connectionService.lease()
			]).catch(error => {
				// Assert:
				expect(error.message).to.equal('connection failed');
			});

			// unblocks the first promise (simulates delayed authorization failure)
			sockets[0].fireEvent('close');

			return leasePromises;
		});
	});
});
