const BasicNetworkTimestamp = require('../NetworkTimestamp').NetworkTimestamp;
const { NetworkTimestampDatetimeConverter } = require('../NetworkTimestamp');

const createCoverter = epochTime => {
	const EPOCH_TIME = new Date(Date.UTC(2021, 3, 16, 0, 6, 25));
	return new NetworkTimestampDatetimeConverter(epochTime || EPOCH_TIME, 'milliseconds');
};

/**
 * Represents a symbol network timestamp with millisecond resolution.
 */
class NetworkTimestamp extends BasicNetworkTimestamp {
	/**
	 * Adds a specified number of milliseconds to this timestamp.
	 * @param {number} count Number of milliseconds to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of milliseconds past this timestamp.
	 */
	addMilliseconds(count) {
		return new NetworkTimestamp(this.timestamp + count);
	}

	/**
	 * Adds a specified number of seconds to this timestamp.
	 * @override
	 * @param {number} count Number of seconds to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of seconds past this timestamp.
	 */
	addSeconds(count) {
		return this.addMilliseconds(1000 * count);
	}

	/**
	 * Converts this timestamp to a datetime.
	 * @param {Date} epochTime Custom network start time (undefined to use mainnet start time).
	 * @returns {Date} Date representation of this timestamp.
	 */
	toDatetime(epochTime = undefined) {
		return createCoverter(epochTime).toDatetime(this.timestamp);
	}

	/**
	 * Creates a network timestamp from a datetime.
	 * @param {Date} referenceDatetime Reference date.
	 * @param {Date} epochTime Custom network start time (undefined to use mainnet start time).
	 * @returns {NetworkTimestamp} Network timestamp representing the reference date.
	 */
	static fromDatetime(referenceDatetime, epochTime = undefined) {
		return new NetworkTimestamp(createCoverter(epochTime).toDifference(referenceDatetime));
	}
}

module.exports = { NetworkTimestamp };
