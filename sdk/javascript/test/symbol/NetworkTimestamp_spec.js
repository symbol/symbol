const { NetworkTimestamp } = require('../../src/symbol/NetworkTimestamp');
const { runBasicNetworkTimestampTests } = require('../test/networkTimestampTests');
const { expect } = require('chai');

describe('NetworkTimestamp (Symbol)', () => {
	runBasicNetworkTimestampTests({
		NetworkTimestamp,
		epochTime: new Date(Date.UTC(2021, 2, 16, 0, 6, 25)),
		timeUnitMultiplier: 1
	});

	const runTestCases = (wrapInt, postfix) => {
		it(`can add milliseconds (${postfix})`, () => {
			// Arrange:
			const timestamp = new NetworkTimestamp(wrapInt(100));

			// Act:
			const newTimestamp = timestamp.addMilliseconds(wrapInt(50));

			// Assert:
			expect(timestamp.timestamp).to.equal(100n);
			expect(newTimestamp.timestamp).to.equal(100n + 50n);
		});

		it(`can add seconds (${postfix})`, () => {
			// Arrange:
			const timestamp = new NetworkTimestamp(wrapInt(100));

			// Act:
			const newTimestamp = timestamp.addSeconds(wrapInt(50));

			// Assert:
			expect(timestamp.timestamp).to.equal(100n);
			expect(newTimestamp.timestamp).to.equal(100n + (50n * 1000n));
		});
	};

	runTestCases(n => n, 'Number');
	runTestCases(n => BigInt(n), 'BigInt');
});
