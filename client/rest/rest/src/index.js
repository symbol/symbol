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

const catapult = require('catapult-sdk');
const fs = require('fs');
const { createConnection } = require('net');
const winston = require('winston');
const { createConnectionService } = require('./connection/connectionService');
const { createZmqConnectionService } = require('./connection/zmqService');
const CatapultDb = require('./db/CatapultDb');
const dbFormattingRules = require('./db/dbFormattingRules');
const routeSystem = require('./plugins/routeSystem');
const allRoutes = require('./routes/allRoutes');
const bootstrapper = require('./server/bootstrapper');
const formatters = require('./server/formatters');
const messageFormattingRules = require('./server/messageFormattingRules');

const configureLogging = config => {
	winston.remove(winston.transports.Console);
	if ('production' !== process.env.NODE_ENV)
		winston.add(winston.transports.Console, config.console);

	winston.add(winston.transports.File, config.file);
};

const loadConfig = () => {
	let configFiles = process.argv.slice(2);
	if (0 === configFiles.length)
		configFiles = ['../resources/rest.json'];

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

const connectToDbWithRetry = (db, config) => catapult.utils.future.makeRetryable(
	() => db.connect(config.url, config.name),
	config.maxConnectionAttempts,
	(i, err) => {
		const waitTime = (2 ** (i - 1)) * config.baseRetryDelay;
		winston.warn(`db connection failed, retrying in ${waitTime}ms`, err);
		return waitTime;
	}
);

const createServer = config => {
	const modelSystem = catapult.plugins.catapultModelSystem.configure(config.extensions, {
		json: dbFormattingRules,
		ws: messageFormattingRules
	});
	return {
		server: bootstrapper.createServer(config.crossDomainHttpMethods, formatters.create(modelSystem.formatters)),
		codec: modelSystem.codec
	};
};

const registerRoutes = (server, db, services) => {
	// 1. create a services view for extension routes
	const servicesView = {
		config: {
			network: services.config.network,
			pageSize: {
				min: services.config.db.pageSizeMin,
				max: services.config.db.pageSizeMax,
				step: services.config.db.pageSizeStep
			},
			apiNode: services.config.apiNode
		},
		connections: services.connectionService
	};

	// 2. configure extension routes
	const { transactionStates, messageChannelDescriptors } = routeSystem.configure(services.config.extensions, server, db, servicesView);

	// 3. augment services with extension-dependent config and services
	servicesView.config.transactionStates = transactionStates;
	servicesView.zmqService = createZmqConnectionService(services.config.websocket.mq, services.codec, messageChannelDescriptors, winston);

	// 4. configure basic routes
	allRoutes.register(server, db, servicesView);
};

(() => {
	const config = loadConfig();
	configureLogging(config.logging);
	winston.verbose('finished loading rest server config', config);

	const network = catapult.model.networkInfo.networks[config.network.name];
	if (!network) {
		winston.error(`no network found with name: '${config.network.name}'`);
		return;
	}

	const serviceManager = createServiceManager();
	const db = new CatapultDb({
		networkId: network.id,
		pageSizeMin: config.db.pageSizeMin,
		pageSizeMax: config.db.pageSizeMax
	});

	serviceManager.pushService(db, 'close');

	winston.info(`connecting to ${config.db.url} (database:${config.db.name})`);
	connectToDbWithRetry(db, config.db)
		.then(() => {
			winston.info('registering routes');
			const serverAndCodec = createServer(config);
			const { server } = serverAndCodec;
			serviceManager.pushService(server, 'close');

			const connectionService = createConnectionService(config, createConnection, catapult.auth.createAuthPromise, winston.verbose);
			registerRoutes(server, db, { codec: serverAndCodec.codec, config, connectionService });

			winston.info(`listening on port ${config.port}`);
			server.listen(config.port);
		})
		.catch(err => {
			winston.error('rest server is exiting due to error', err);
			serviceManager.stopAll();
		});

	process.on('SIGINT', () => {
		winston.info('SIGINT detected, shutting down rest server');
		serviceManager.stopAll();
	});
})();
