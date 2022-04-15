const { NetworkTimestamp } = require('../../src/nem/NetworkTimestamp');
const { runBasicNetworkTimestampTests } = require('../test/networkTimestampTests');
const { expect } = require('chai');

describe('NetworkTimestamp (NEM)', () => {
	runBasicNetworkTimestampTests({
		NetworkTimestamp,
		epochTime: new Date(Date.UTC(2015, 2, 29, 0, 6, 25)),
		timeUnitMultiplier: 1000
	});

	it('can add seconds', () => {
		// Arrange:
		const timestamp = new NetworkTimestamp(100);

		// Act:
		const newTimestamp = timestamp.addSeconds(50);

		// Assert:
		expect(timestamp.timestamp).to.equal(100);
		expect(newTimestamp.timestamp).to.equal(100 + 50);
	});
});
