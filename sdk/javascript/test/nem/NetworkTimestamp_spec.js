const { NetworkTimestamp } = require('../../src/nem/NetworkTimestamp');
const { runBasicNetworkTimestampTests } = require('../test/networkTimestampTests');
const { expect } = require('chai');

describe('NetworkTimestamp (NEM)', () => {
	runBasicNetworkTimestampTests({
		NetworkTimestamp,
		epochTime: new Date(Date.UTC(2015, 2, 29, 0, 6, 25)),
		timeUnitMultiplier: 1000
	});

	const runTestCases = (wrapInt, postfix) => {
		it(`can add seconds (${postfix})`, () => {
			// Arrange:
			const timestamp = new NetworkTimestamp(wrapInt(100));

			// Act:
			const newTimestamp = timestamp.addSeconds(wrapInt(50));

			// Assert:
			expect(timestamp.timestamp).to.equal(100n);
			expect(newTimestamp.timestamp).to.equal(100n + 50n);
		});
	};

	runTestCases(n => n, 'Number');
	runTestCases(n => BigInt(n), 'BigInt');
});
