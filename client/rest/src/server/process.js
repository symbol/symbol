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

import catapult from '../catapult-sdk/index.js';
import winston from 'winston';
import fs from 'fs';

const createLoggingTransportConfiguration = loggingConfig => {
	const transportConfig = Object.assign({}, loggingConfig);

	// map specified formats into a winston function
	delete transportConfig.formats;
	const logFormatters = loggingConfig.formats.map(name => winston.format[name]());
	transportConfig.format = winston.format.combine(...logFormatters);
	return transportConfig;
};

const configureLogging = config => {
	const transports = [new winston.transports.File(createLoggingTransportConfiguration(config.file))];
	if ('production' !== process.env.NODE_ENV)
		transports.push(new winston.transports.Console(createLoggingTransportConfiguration(config.console)));

	// configure default logger so that it adds timestamp to all logs
	winston.configure({
		format: winston.format.timestamp(),
		transports
	});
};

const validateConfig = config => {
	if (config.crossDomain && (!config.crossDomain.allowedHosts || !config.crossDomain.allowedMethods))
		throw Error('provided CORS configuration is incomplete');
};

const loadConfig = configFiles => {
	let config;
	configFiles.forEach(configFile => {
		winston.info(`loading config from ${configFile}`);
		const partialConfig = JSON.parse(fs.readFileSync(configFile, 'utf8'));
		if (config) {
			// override config
			catapult.utils.objects.checkSchemaAgainstTemplate(config, partialConfig);
			catapult.utils.objects.deepAssign(config, partialConfig);
		} else {
			// primary config
			config = partialConfig;
		}
	});

	validateConfig(config);
	return config;
};

const createServiceManager = () => {
	const shutdownHandlers = [];
	return {
		pushService: (object, shutdownHandlerName) => {
			shutdownHandlers.push(() => { object[shutdownHandlerName](); });
		},
		stopAll: () => {
			while (0 < shutdownHandlers.length)
				shutdownHandlers.pop()();
		}
	};
};

/**
 * Sets up a process shell and hosts logic defined within a handler.
 * @param {Array<string>} configFiles Names of configuration files to load.
 * @param {Function} handler Main process handler.
 * @returns {Promise} Promise that is tied to the process lifetime.
 */
export default (configFiles, handler) => {
	const config = loadConfig(configFiles);

	configureLogging(config.logging);
	winston.verbose('finished loading rest server config', config);

	const serviceManager = createServiceManager();

	process.on('SIGINT', () => {
		winston.info('SIGINT detected, shutting down rest server');
		serviceManager.stopAll();
	});

	const handleError = err => {
		winston.error('rest server is exiting due to error', err);
		serviceManager.stopAll();
	};

	try {
		return Promise.resolve(handler(config, serviceManager)).catch(handleError);
	} catch (err) {
		handleError(err);
		return Promise.resolve(); // suppress error
	}
};
