const { NetworkTimestamp } = require('../../src/symbol/NetworkTimestamp');
const { runBasicNetworkTimestampTests } = require('../test/networkTimestampTests');
const { expect } = require('chai');

describe('NetworkTimestamp (Symbol)', () => {
	runBasicNetworkTimestampTests({
		NetworkTimestamp,
		epochTime: new Date(Date.UTC(2021, 2, 16, 0, 6, 25)),
		timeUnitMultiplier: 1
	});

	it('can add milliseconds', () => {
		// Arrange:
		const timestamp = new NetworkTimestamp(100);

		// Act:
		const newTimestamp = timestamp.addMilliseconds(50);

		// Assert:
		expect(timestamp.timestamp).to.equal(100);
		expect(newTimestamp.timestamp).to.equal(100 + 50);
	});

	it('can add seconds', () => {
		// Arrange:
		const timestamp = new NetworkTimestamp(100);

		// Act:
		const newTimestamp = timestamp.addSeconds(50);

		// Assert:
		expect(timestamp.timestamp).to.equal(100);
		expect(newTimestamp.timestamp).to.equal(100 + (50 * 1000));
	});
});
