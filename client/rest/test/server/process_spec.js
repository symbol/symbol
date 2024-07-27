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

import runProcess from '../../src/server/process.js';
import { expect } from 'chai';
import tmp from 'tmp';
import winston from 'winston';
import fs from 'fs';

describe('process', () => {
	// region utils

	const DEFAULT_CONFIG_FILENAME = './resources/rest.rosetta.json';

	const runProcessWithTempLoggingFile = (configFiles, handler) => {
		const tempConfigFile = tmp.fileSync();
		const tempLoggingFile = tmp.fileSync();
		fs.writeFileSync(tempConfigFile.name, `{ "logging": { "file": { "filename": "${tempLoggingFile.name}" } } }`);

		runProcess([...configFiles, tempConfigFile.name], handler);

		return {
			tempConfigFilename: tempConfigFile.name,
			tempLoggingFilename: tempLoggingFile.name
		};
	};

	const runProcessWithTempLoggingFileAsync = async (configFiles, handler) => {
		const tempConfigFile = tmp.fileSync();
		const tempLoggingFile = tmp.fileSync();
		fs.writeFileSync(tempConfigFile.name, `{ "logging": { "file": { "filename": "${tempLoggingFile.name}" } } }`);

		await runProcess([...configFiles, tempConfigFile.name], handler);

		return {
			tempConfigFilename: tempConfigFile.name,
			tempLoggingFilename: tempLoggingFile.name
		};
	};

	const sleep = sleepMilliseconds => new Promise(resolve => { setTimeout(resolve, sleepMilliseconds); });

	const assertMessageLoggedExact = async (loggingFilename, expectedMessage) => {
		// read contents of log file
		const loggingFileContents = await new Promise(resolve => {
			fs.readFile(loggingFilename, { encoding: 'utf8', flag: 'r' }, async (err, data) => {
				resolve(data);
			});
		});

		if ('' === loggingFileContents) {
			// pause and retry if log hasn't been flushed to disk yet
			await sleep(100);
			await assertMessageLoggedExact(loggingFilename, expectedMessage);
			return;
		}

		// assert expected log message is included
		expect(loggingFileContents.includes(`message: '${expectedMessage}'`)).to.equal(true);
	};

	afterEach(async () => {
		// unregister the logging transports configured by runProcess
		// there is a race condtion (transports shouldn't be closed before they are flushed)
		// so, add a small pause before and after clear
		await sleep(100);
		winston.clear();
		await sleep(100);
	});

	// endregion

	// region config

	describe('config', () => {
		it('fails when override config doesn\'t match schema', () => {
			// Arrange:
			const tempConfigFile = tmp.fileSync();
			fs.writeFileSync(tempConfigFile.name, '{ "network": { "foo": "bar" } }');

			// Act + Assert:
			expect(() => runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME, tempConfigFile.name], () => {}))
				.to.throw('unknown \'foo\' key in config');
		});

		const createConfigWithCrossDomain = crossDomainBody =>
			`{ "crossDomain": ${crossDomainBody}, "logging": { "file": { "filename": "" } } }`;

		it('fails when crossDomain is set without allowedHosts', () => {
			// Arrange:
			const tempConfigFile = tmp.fileSync();
			fs.writeFileSync(tempConfigFile.name, createConfigWithCrossDomain('{ "allowedMethods": [] }'));

			// Act + Assert:
			expect(() => runProcessWithTempLoggingFile([tempConfigFile.name], () => {}))
				.to.throw('provided CORS configuration is incomplete');
		});

		it('fails when crossDomain is set without allowedMethods', () => {
			// Arrange:
			const tempConfigFile = tmp.fileSync();
			fs.writeFileSync(tempConfigFile.name, createConfigWithCrossDomain('{ "allowedHosts": [] }'));

			// Act + Assert:
			expect(() => runProcessWithTempLoggingFile([tempConfigFile.name], () => {}))
				.to.throw('provided CORS configuration is incomplete');
		});

		it('succeeds when crossDomain is set with full configuration', () => {
			// Arrange: make sure cross domain is set
			const tempConfigFile = tmp.fileSync();
			fs.writeFileSync(tempConfigFile.name, createConfigWithCrossDomain('{}'));

			// Act + Assert:
			expect(() => runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME, tempConfigFile.name], () => {})).to.not.throw();
		});
	});

	// endregion

	// region logging

	describe('logging', () => {
		it('registers two transports', () => {
			// Sanity:
			expect(winston.default.transports.length).to.equal(0);

			// Act:
			runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME], () => {});

			// Assert:
			expect(winston.default.transports.length).to.equal(2);
		});

		it('captures log messages', async () => {
			// Act:
			const { tempLoggingFilename } = runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME], () => {});

			// Assert: expected log message is included
			await assertMessageLoggedExact(tempLoggingFilename, 'finished loading rest server config');
		});
	});

	// endregion

	// region handler

	describe('handler', () => {
		it('receives parsed config', () => {
			// Arrange:
			const tempConfigFile = tmp.fileSync();
			fs.writeFileSync(tempConfigFile.name, '{ "network": { "name": "foobar" } }');

			// Act:
			let handlerConfig;
			runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME, tempConfigFile.name], config => {
				handlerConfig = config;
			});

			// Assert: check network config composed of custom and default property
			expect(handlerConfig.network).to.deep.equal({
				name: 'foobar',
				description: 'catapult public test network'
			});
		});

		const createMockServices = () => {
			let serviceAlphaStopCalls = 0;
			const serviceAlpha = {
				stop: () => {
					serviceAlphaStopCalls++;
				}
			};

			let serviceBetaShutdownCalls = 0;
			const serviceBeta = {
				shutdown: () => {
					serviceBetaShutdownCalls++;
				}
			};

			return {
				pushAll: serviceManager => {
					serviceManager.pushService(serviceAlpha, 'stop');
					serviceManager.pushService(serviceBeta, 'shutdown');
				},

				checkCallCounts: expectedCallCount => {
					expect(serviceAlphaStopCalls).to.equal(expectedCallCount);
					expect(serviceBetaShutdownCalls).to.equal(expectedCallCount);
				}
			};
		};

		it('can register services for shutdown on error (sync)', async () => {
			// Arrange:
			const services = createMockServices();

			// Act:
			const { tempLoggingFilename } = runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME], (config, serviceManager) => {
				services.pushAll(serviceManager);
				throw Error('handler error');
			});

			// Assert:
			services.checkCallCounts(1);
			await assertMessageLoggedExact(tempLoggingFilename, 'rest server is exiting due to error handler error');
		});

		it('can register services for shutdown on error (async)', async () => {
			// Arrange:
			const services = createMockServices();

			// Act:
			const { tempLoggingFilename } = await runProcessWithTempLoggingFileAsync(
				[DEFAULT_CONFIG_FILENAME],
				async (config, serviceManager) => {
					services.pushAll(serviceManager);
					await sleep(100);
					throw Error('handler error');
				}
			);

			// Assert:
			services.checkCallCounts(1);
			await assertMessageLoggedExact(tempLoggingFilename, 'rest server is exiting due to error handler error');
		});

		it('can register services for shutdown on SIGINT', async () => {
			// Arrange:
			const services = createMockServices();

			// Act:
			const { tempLoggingFilename } = runProcessWithTempLoggingFile([DEFAULT_CONFIG_FILENAME], (config, serviceManager) => {
				services.pushAll(serviceManager);
				process.emit('SIGINT');
			});

			// Assert:
			services.checkCallCounts(1);
			await assertMessageLoggedExact(tempLoggingFilename, 'SIGINT detected, shutting down rest server');
		});
	});

	// endregion
});
