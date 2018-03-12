const { expect } = require('chai');
const aggregate = require('../../src/plugins/aggregate');
const pluginTest = require('./utils/pluginTestUtils');
const test = require('../routes/utils/routeTestUtils');

describe('aggregate plugin', () => {
	pluginTest.assertThat.pluginDoesNotCreateDb(aggregate);

	describe('register transaction states', () => {
		it('registers partial state', () => {
			// Arrange:
			const states = [];

			// Act:
			aggregate.registerTransactionStates(states);

			// Assert:
			expect(states.length).to.equal(1);
			expect(states[0]).to.deep.equal({ friendlyName: 'partial', dbPostfix: 'Partial', routePostfix: '/partial' });
		});
	});

	describe('register message channels', () => {
		const registerAndExtractChannelDescriptor = channelDescriptorName => {
			// Arrange:
			const channelDescriptors = [];
			const builder = { add: (name, markerChar, handler) => { channelDescriptors.push({ name, markerChar, handler }); } };

			// Act:
			aggregate.registerMessageChannels(builder);
			const channelDescriptor = channelDescriptors.find(descriptor => channelDescriptorName === descriptor.name);

			// Sanity:
			expect(channelDescriptors.length).to.equal(3);
			expect(channelDescriptor).to.not.equal(undefined);
			return channelDescriptor;
		};

		it('registers partialAdded', () => {
			// Act:
			const descriptor = registerAndExtractChannelDescriptor('partialAdded');

			// Assert:
			expect(descriptor).to.deep.equal({ name: 'partialAdded', markerChar: 'p', handler: 'transaction' });
		});

		it('registers partialRemoved', () => {
			// Act:
			const descriptor = registerAndExtractChannelDescriptor('partialRemoved');

			// Assert:
			expect(descriptor).to.deep.equal({ name: 'partialRemoved', markerChar: 'q', handler: 'transactionHash' });
		});

		it('registers cosignature', () => {
			// Act:
			const descriptor = registerAndExtractChannelDescriptor('cosignature');

			// Assert:
			expect(descriptor.name).to.equal('cosignature');
			expect(descriptor.markerChar).to.equal('c');
		});

		it('registers cosignature with handler that forwards to emit callback', () => {
			// Arrange:
			const emitted = [];
			const { handler } = registerAndExtractChannelDescriptor('cosignature');

			// Act:
			const buffer = Buffer.concat([
				Buffer.alloc(test.constants.sizes.signer, 33),
				Buffer.alloc(test.constants.sizes.signature, 44),
				Buffer.alloc(test.constants.sizes.hash, 55)
			]);
			handler({}, eventData => emitted.push(eventData))(22, buffer, 99);

			// Assert:
			// - 22 is a "topic" so it's not forwarded
			// - trailing param 99 should be ignored
			expect(emitted.length).to.equal(1);
			expect(emitted[0]).to.deep.equal({
				type: 'aggregate.cosignature',
				payload: {
					signer: Buffer.alloc(test.constants.sizes.signer, 33),
					signature: Buffer.alloc(test.constants.sizes.signature, 44),
					parentHash: Buffer.alloc(test.constants.sizes.hash, 55)
				}
			});
		});
	});

	describe('register routes', () => {
		it('registers aggregate PUT routes', () => {
			// Arrange:
			const routes = [];
			const server = test.setup.createCapturingMockServer('put', routes);

			// Act:
			aggregate.registerRoutes(server, {}, {});

			// Assert:
			test.assert.assertRoutes(routes, [
				'/transaction/cosignature',
				'/transaction/partial'
			]);
		});
	});
});
