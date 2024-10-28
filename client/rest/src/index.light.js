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
import routeSystem from './plugins/routeSystem.js';
import bootstrapper from './server/bootstrapper.js';
import formatters from './server/formatters.js';
import messageFormattingRules from './server/messageFormattingRules.js';
import runProcess from './server/process.js';
import sshpk from 'sshpk';
import winston from 'winston';
import fs from 'fs';

const createServer = config => {
	const modelSystem = catapult.plugins.catapultModelSystem.configure([], {
		ws: messageFormattingRules
	});

	return bootstrapper.createServer(config, formatters.create(modelSystem.formatters), config.throttling);
};

const registerRoutes = (server, db, services) => {
	// 1. create a services view for extension routes
	const servicesView = {
		config: {
			network: services.config.network,
			apiNode: services.config.apiNode,
			deployment: services.config.deployment
		},
		connections: services.connectionService
	};

	// 2. configure extension routes
	routeSystem.configure(services.config.routeExtensions, server, db, servicesView);
};

(() => {
	let configFiles = process.argv.slice(2);
	if (0 === configFiles.length)
		configFiles = ['../resources/rest.light.json'];

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

		winston.info('registering routes');
		const server = createServer(config);
		serviceManager.pushService(server, 'close');

		const connectionConfig = {
			apiNode: config.apiNode
		};
		const connectionService = createConnectionService(connectionConfig, winston.verbose);
		registerRoutes(server, undefined, { config, connectionService });

		winston.info(`listening on port ${config.port}`);
		server.listen(config.port);
	});
})();
