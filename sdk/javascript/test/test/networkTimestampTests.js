const { expect } = require('chai');

const CUSTOM_EPOCH_TIME = new Date(Date.UTC(2021, 1, 1));

const runBasicNetworkTimestampTests = testDescriptor => {
	const getTimeUnits = value => value * testDescriptor.timeUnitMultiplier;

	// region toDatetime

	it('can convert epochal timestamp to datetime', () => {
		// Act:
		const utcTimestamp = new testDescriptor.NetworkTimestamp(0).toDatetime();

		// Assert:
		expect(utcTimestamp.getTime()).to.equal(testDescriptor.epochTime.getTime());
	});

	it('can convert non epochal timestamp to datetime', () => {
		// Act:
		const utcTimestamp = new testDescriptor.NetworkTimestamp(123).toDatetime();

		// Assert:
		expect(utcTimestamp.getTime()).to.equal(testDescriptor.epochTime.getTime() + getTimeUnits(123));
	});

	it('can convert epochal timestamp to datetime (custom epoch)', () => {
		// Act:
		const utcTimestamp = new testDescriptor.NetworkTimestamp(0).toDatetime(CUSTOM_EPOCH_TIME);

		// Assert:
		expect(utcTimestamp.getTime()).to.equal(CUSTOM_EPOCH_TIME.getTime());
	});

	it('can convert non epochal timestamp to datetime (custom epoch)', () => {
		// Act:
		const utcTimestamp = new testDescriptor.NetworkTimestamp(123).toDatetime(CUSTOM_EPOCH_TIME);

		// Assert:
		expect(utcTimestamp.getTime()).to.equal(CUSTOM_EPOCH_TIME.getTime() + getTimeUnits(123));
	});

	// endregion

	// region fromDatetime

	it('can convert datetime to epochal timestamp', () => {
		// Act:
		const timestamp = testDescriptor.NetworkTimestamp.fromDatetime(testDescriptor.epochTime);

		// Assert:
		expect(timestamp.isEpochal).to.equal(true);
		expect(timestamp.timestamp).to.equal(0n);
	});

	it('can convert datetime to non epochal timestamp', () => {
		// Act:
		const { fromDatetime } = testDescriptor.NetworkTimestamp;
		const timestamp = fromDatetime(new Date(testDescriptor.epochTime.getTime() + getTimeUnits(123)));

		// Assert:
		expect(timestamp.isEpochal).to.equal(false);
		expect(timestamp.timestamp).to.equal(123n);
	});

	it('can convert datetime to epochal timestamp (custom epoch)', () => {
		// Act:
		const timestamp = testDescriptor.NetworkTimestamp.fromDatetime(CUSTOM_EPOCH_TIME, CUSTOM_EPOCH_TIME);

		// Assert:
		expect(timestamp.isEpochal).to.equal(true);
		expect(timestamp.timestamp).to.equal(0n);
	});

	it('can convert datetime to non epochal timestamp (custom epoch)', () => {
		// Act:
		const { fromDatetime } = testDescriptor.NetworkTimestamp;
		const timestamp = fromDatetime(new Date(CUSTOM_EPOCH_TIME.getTime() + getTimeUnits(123)), CUSTOM_EPOCH_TIME);

		// Assert:
		expect(timestamp.isEpochal).to.equal(false);
		expect(timestamp.timestamp).to.equal(123n);
	});

	// endregion
};

module.exports = { runBasicNetworkTimestampTests };
