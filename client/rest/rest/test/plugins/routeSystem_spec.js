import { expect } from 'chai';
import routeSystem from '../../src/plugins/routeSystem';
import test from '../routes/utils/routeTestUtils';

describe('route system', () => {
	it('cannot register unknown extension', () => {
		// Act:
		expect(() => routeSystem.configure(['transfer', 'foo', 'namespace'])).to.throw('plugin \'foo\' not supported');
	});

	it('has support for all plugins', () => {
		// Act:
		const supportedPluginNames = routeSystem.supportedPluginNames();

		// Assert:
		expect(supportedPluginNames).to.deep.equal(['aggregate', 'multisig', 'namespace', 'transfer']);
	});

	it('does not register default routes', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('get', routes);

		// Act:
		routeSystem.configure([], server);

		// Assert:
		expect(routes.length).to.equal(0);
	});

	it('can register single extension', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('get', routes);
		const db = { namespaceById: () => Promise.resolve({}) };

		// Act:
		routeSystem.configure(['namespace'], server, db);

		// Assert:
		expect(routes).to.include('/namespace/id/:id');
	});

	it('can register multiple extensions', () => {
		// Arrange:
		const routes = [];
		const server = test.setup.createCapturingMockServer('get', routes);
		const db = { namespaceById: () => Promise.resolve({}) };

		// Act:
		routeSystem.configure(['namespace', 'transfer'], server, db);

		// Assert:
		expect(routes).to.include('/namespace/id/:id');
	});
});
