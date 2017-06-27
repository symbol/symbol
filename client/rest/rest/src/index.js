import catapult from 'catapult-sdk';
import fs from 'fs';
import { createConnection } from 'net';
import winston from 'winston';
import createConnectionService from './connection/connectionService';
import CatapultDb from './db/CatapultDb';
import connector from './db/connector';
import formattingRules from './db/formattingRules';
import entityEmitterFactory from './db/entityEmitterFactory';
import routeSystem from './plugins/routeSystem';
import allRoutes from './routes/allRoutes';
import bootstrapper from './server/bootstrapper';
import formatters from './server/formatters';

const createAuthPromise = catapult.auth.createAuthPromise;
const objects = catapult.utils.objects;

function configureLogging(config) {
	winston.remove(winston.transports.Console);
	winston.add(winston.transports.Console, config.console);
	winston.add(winston.transports.File, config.file);
}

function loadConfig() {
	let configFiles = process.argv.slice(2);
	if (0 === configFiles.length)
		configFiles = ['../resources/rest.json'];

	let config;
	for (const configFile of configFiles) {
		winston.info(`loading config from ${configFile}`);
		const partialConfig = JSON.parse(fs.readFileSync(configFile, 'utf8'));

		if (config) {
			// override config
			objects.checkSchemaAgainstTemplate(config, partialConfig);
			objects.deepAssign(config, partialConfig);
		} else {
			// primary config
			config = partialConfig;
		}
	}

	return config;
}

function createServiceManager() {
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
}

function connectToDbWithRetry(db, config) {
	return catapult.utils.future.makeRetryable(
		() => db.connect(config.url, config.name),
		config.maxConnectionAttempts,
		(i, err) => {
			const waitTime = Math.pow(2, i - 1) * config.baseRetryDelay;
			winston.warn(`db connection failed, retrying in ${waitTime}ms: ${err.message}`);
			return waitTime;
		});
}

function createServer(config) {
	const modelSystem = catapult.plugins.catapultModelSystem.configure(config.extensions, formattingRules);
	return bootstrapper.createServer(config.crossDomainHttpMethods, formatters.create(modelSystem.formatter));
}

function registerRoutes(config, server, db, services) {
	allRoutes.register(server, db, services);
	routeSystem.configure(config.extensions, server, db);
}

(function () {
	const config = loadConfig();
	configureLogging(config.logging);
	winston.verbose('finished loading rest server config', config);

	const serviceManager = createServiceManager();
	const db = new CatapultDb({
		networkId: config.networkId,
		pageSizeMin: config.db.pageSizeMin,
		pageSizeMax: config.db.pageSizeMax
	});

	serviceManager.pushService(db, 'close');

	winston.info(`connecting to ${config.db.url} (database:${config.db.name})`);
	connectToDbWithRetry(db, config.db)
		.then(() => entityEmitterFactory.createEntityEmitter(query =>
			connector.startTailingOplog(config.db.url, query)
				.then(emitterConnection => {
					serviceManager.pushService(emitterConnection.emitter, 'stop');
					serviceManager.pushService(emitterConnection.db, 'close');
					return emitterConnection.emitter;
				})))
		.then(entityEmitter => {
			winston.info('registering routes');
			const server = createServer(config);
			serviceManager.pushService(server, 'close');

			const connectionService = createConnectionService(config, createConnection, createAuthPromise, winston.verbose);
			const services = {
				connections: connectionService,
				entityEmitter
			};

			registerRoutes(config, server, db, services);

			winston.info(`listening on port ${config.port}`);
			server.listen(config.port);
		})
		.catch(err => {
			winston.error(`rest server is exiting due to error: ${err.message}`);
			serviceManager.stopAll();
		});

	process.on('SIGINT', () => {
		winston.info('SIGINT detected, shutting down rest server');
		serviceManager.stopAll();
	});
})();
