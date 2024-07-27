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

import routeSystem from './plugins/routeSystem.js';
import bootstrapper from './server/bootstrapper.js';
import formatters from './server/formatters.js';
import runProcess from './server/process.js';
import winston from 'winston';

// pass empty json because no custom object formatters are needed
const createServer = config => bootstrapper.createServer(config, formatters.create({ json: {} }, config.throttling));

const registerRoutes = (server, db, services) => {
	// 1. create a services view for extension routes
	const servicesView = {
		config: {
			network: services.config.network,
			restEndpoint: services.config.restEndpoint,
			rosetta: services.config.rosetta
		}
	};

	// 2. configure extension routes
	routeSystem.configure(services.config.routeExtensions, server, db, servicesView);
};

(() => {
	let configFiles = process.argv.slice(2);
	if (0 === configFiles.length)
		configFiles = ['../resources/rest.rosetta.json'];

	runProcess(configFiles, (config, serviceManager) => {
		winston.info('registering routes');
		const server = createServer(config);
		serviceManager.pushService(server, 'close');

		registerRoutes(server, undefined, { config });

		winston.info(`listening on port ${config.port}`);
		server.listen(config.port);
	});
})();
