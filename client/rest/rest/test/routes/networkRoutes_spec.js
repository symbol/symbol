const { expect } = require('chai');
const networkRoutes = require('../../src/routes/networkRoutes');
const test = require('./utils/routeTestUtils');

describe('network routes', () => {
	describe('get', () => {
		it('can retrieve network information', () => {
			// Act:
			const config = { network: { name: 'foo', head: 'bar' } };
			return test.route.prepareExecuteRoute(networkRoutes.register, '/network', 'get', {}, {}, config, routeContext => {
				// - invoke route synchronously
				routeContext.routeInvoker();

				// Assert:
				expect(routeContext.numNextCalls, 'next should be called once').to.equal(1);
				expect(routeContext.responses.length, 'single response is expected').to.equal(1);
				expect(routeContext.redirects.length, 'no redirects are expected').to.equal(0);

				// - no type information because formatting is completely bypassed
				const response = routeContext.responses[0];
				expect(response).to.deep.equal({ name: 'foo', head: 'bar' });
				expect(response).to.equal(config.network);
			});
		});
	});
});
