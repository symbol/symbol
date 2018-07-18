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

const { expect } = require('chai');

const networkRoutes = require('../../src/routes/networkRoutes');
const test = require('./utils/routeTestUtils');

describe('network routes', () => {
	describe('get', () => {
		it('can retrieve network', () => {
			// Act:
			const services = { config: { network: { name: 'foo', head: 'bar' } } };
			return test.route.prepareExecuteRoute(networkRoutes.register, '/network', 'get', {}, {}, services, routeContext => {
				// - invoke route synchronously
				routeContext.routeInvoker();

				// Assert:
				expect(routeContext.numNextCalls, 'next should be called once').to.equal(1);
				expect(routeContext.responses.length, 'single response is expected').to.equal(1);
				expect(routeContext.redirects.length, 'no redirects are expected').to.equal(0);

				// - no type information because formatting is completely bypassed
				const response = routeContext.responses[0];
				expect(response).to.deep.equal({ name: 'foo', head: 'bar' });
				expect(response).to.equal(services.config.network);
			});
		});
	});
});
