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

import catapult from './catapult-sdk/index.js';
import createConnectionService from './connection/connectionService.js';
import createZmqConnectionService from './connection/zmqService.js';
import CatapultDb from './db/CatapultDb.js';
import dbFormattingRules from './db/dbFormattingRules.js';
import routeSystem from './plugins/routeSystem.js';
import allRoutes from './routes/allRoutes.js';
import bootstrapper from './server/bootstrapper.js';
import formatters from './server/formatters.js';
import messageFormattingRules from './server/messageFormattingRules.js';
import runProcess from './server/process.js';
import sshpk from 'sshpk';
import { NetworkLocator } from 'symbol-sdk';
import { Network } from 'symbol-sdk/symbol';
import winston from 'winston';
import fs from 'fs';

const connectToDbWithRetry = (db, config) => catapult.utils.future.makeRetryable(
	() => db.connect(config.url, config.name, config.connectionPoolSize, config.connectionTimeout),
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
	return bootstrapper.createServer(config, formatters.create(modelSystem.formatters), config.throttling);
};

const registerRoutes = (server, db, services) => {
	// 1. create a services view for extension routes
	const servicesView = {
		config: {
			network: services.config.network,
			restEndpoint: `${services.config.protocol}://localhost:${services.config.port}`,
			pageSize: {
				min: services.config.db.pageSizeMin || 10,
				max: services.config.db.pageSizeMax || 100,
				default: services.config.db.pageSizeDefault || 20
			},
			apiNode: services.config.apiNode,
			websocket: services.config.websocket,
			numBlocksTransactionFeeStats: services.config.numBlocksTransactionFeeStats,
			deployment: services.config.deployment,

			uncirculatingAccountPublicKeys: services.config.uncirculatingAccountPublicKeys,
			nodeMetadata: services.config.nodeMetadata,

			metal: services.config.metal,

			rosetta: services.config.rosetta
		},
		connections: services.connectionService
	};

	// 2. configure extension routes
	const { transactionStates, messageChannelDescriptors } = routeSystem.configure(
		[].concat(services.config.extensions, services.config.routeExtensions),
		server,
		db,
		servicesView
	);

	// 3. augment services with extension-dependent config and services
	servicesView.config.transactionStates = transactionStates;
	servicesView.zmqService = createZmqConnectionService(services.config.websocket.mq, messageChannelDescriptors, winston);

	// 4. configure basic routes
	allRoutes.register(server, db, servicesView);
};

(() => {
	let configFiles = process.argv.slice(2);
	if (0 === configFiles.length)
		configFiles = ['../resources/rest.json'];

	runProcess(configFiles, (config, serviceManager) => {
		// Loading and caching certificates.
		config.apiNode = {
			...config.apiNode,
			certificate: fs.readFileSync(config.apiNode.tlsClientCertificatePath),
			key: fs.readFileSync(config.apiNode.tlsClientKeyPath),
			caCertificate: fs.readFileSync(config.apiNode.tlsCaCertificatePath)
		};
		const nodeCertKey = sshpk.parsePrivateKey(config.apiNode.key);
		config.apiNode.nodePublicKey = nodeCertKey.toPublic().part.A.data;

		const network = NetworkLocator.findByName(Network.NETWORKS, config.network.name);
		const db = new CatapultDb({
			networkId: network.identifier,

			// to be removed when old pagination is not used anymore
			// json settings should also be moved from config.db to config.api or similar
			pageSizeMin: config.db.pageSizeMin,
			pageSizeMax: config.db.pageSizeMax
		});

		serviceManager.pushService(db, 'close');

		winston.info(`connecting to ${config.db.url} (database:${config.db.name})`);
		return connectToDbWithRetry(db, config.db)
			.then(() => {
				winston.info('registering routes');
				const server = createServer(config);
				serviceManager.pushService(server, 'close');

				const connectionConfig = {
					apiNode: config.apiNode
				};
				const connectionService = createConnectionService(connectionConfig, winston.verbose);
				registerRoutes(server, db, { config, connectionService });

				winston.info(`listening on port ${config.port}`);
				server.listen(config.port);
			});
	});
})();
