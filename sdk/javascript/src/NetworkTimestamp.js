/**
 * Represents a network timestamp.
 */
export class NetworkTimestamp {
	/**
	 * Creates a timestamp.
	 * @param {number|bigint} timestamp Raw network timestamp.
	 */
	constructor(timestamp) {
		/**
		 * Underlying timestamp.
		 * @type bigint
		 */
		this.timestamp = BigInt(timestamp);
	}

	/**
	 * Determines if this is the epochal timestamp.
	 */
	get isEpochal() {
		return 0n === this.timestamp;
	}

	/**
	 * Adds a specified number of seconds to this timestamp.
	 * @abstract
	 * @param {number|bigint} count Number of seconds to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of seconds past this timestamp.
	 */
	addSeconds(count) { // eslint-disable-line class-methods-use-this, no-unused-vars
		throw new Error('`addSeconds` must be implemented by concrete class');
	}

	/**
	 * Adds a specified number of minutes to this timestamp.
	 * @param {number|bigint} count Number of minutes to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of minutes past this timestamp.
	 */
	addMinutes(count) {
		return this.addSeconds(60n * BigInt(count));
	}

	/**
	 * Adds a specified number of hours to this timestamp.
	 * @param {number|bigint} count Number of hours to add.
	 * @returns {NetworkTimestamp} New timestamp that is the specified number of hours past this timestamp.
	 */
	addHours(count) {
		return this.addMinutes(60n * BigInt(count));
	}

	/**
	 * Returns string representation of this object.
	 * @returns {string} String representation of this object
	 */
	toString() {
		return this.timestamp.toString();
	}
}

/**
 * Provides utilities for converting between network timestamps and datetimes.
 */
export class NetworkTimestampDatetimeConverter {
	/**
	 * Creates a converter given an epoch and base time units.
	 * @param {Date} epoch Date at which network started.
	 * @param {string} timeUnits Time unit the network uses for progressing.
	 */
	constructor(epoch, timeUnits) {
		/**
		 * Date at which network started
		 * @type Date
		 */
		this.epoch = epoch;

		/**
		 * Number of milliseconds per time unit.
		 * @type number
		 */
		this.timeUnits = {
			hours: 60 * 60 * 1000,
			minutes: 60 * 1000,
			seconds: 1000,
			milliseconds: 1
		}[timeUnits];
	}

	/**
	 * Converts a network timestamp to a datetime.
	 * @param {number} rawTimestamp Raw network timestamp.
	 * @returns {Date} Date representation of the network timestamp.
	 */
	toDatetime(rawTimestamp) {
		return new Date(this.epoch.getTime() + (Number(rawTimestamp) * this.timeUnits));
	}

	/**
	 * Subtracts the network epoch from the reference date.
	 * @param {Date} referenceDatetime Reference date.
	 * @returns {number} Number of network time units between the reference date and the network epoch.
	 */
	toDifference(referenceDatetime) {
		if (referenceDatetime < this.epoch)
			throw RangeError('timestamp cannot be before epoch');

		const subtractDates = (lhs, rhs) => (lhs - rhs);
		return subtractDates(referenceDatetime, this.epoch) / this.timeUnits;
	}
}
