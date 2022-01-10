const { NetworkTimestamp, NetworkTimestampDatetimeConverter } = require('../src/NetworkTimestamp');
const { expect } = require('chai');

describe('NetworkTimestamp', () => {
	class ConcreteNetworkTimestamp extends NetworkTimestamp { // needed because NetworkTimestamp is abstract
		addSeconds(count) {
			return new ConcreteNetworkTimestamp(this.timestamp + (5 * count));
		}
	}

	it('can create epochal timestamp', () => {
		// Act:
		const timestamp = new ConcreteNetworkTimestamp(0);

		// Assert:
		expect(timestamp.isEpochal).to.equal(true);
		expect(timestamp.timestamp).to.equal(0);
	});

	it('can create non epochal timestamp', () => {
		// Act:
		const timestamp = new ConcreteNetworkTimestamp(123);

		// Assert:
		expect(timestamp.isEpochal).to.equal(false);
		expect(timestamp.timestamp).to.equal(123);
	});

	it('can add minutes', () => {
		// Arrange:
		const timestamp = new ConcreteNetworkTimestamp(100);

		// Act:
		const newTimestamp = timestamp.addMinutes(50);

		// Assert:
		expect(timestamp.timestamp).to.equal(100);
		expect(newTimestamp.timestamp).to.equal(100 + (60 * 5 * 50));
	});

	it('can add hours', () => {
		// Arrange:
		const timestamp = new ConcreteNetworkTimestamp(100);

		// Act:
		const newTimestamp = timestamp.addHours(50);

		// Assert:
		expect(timestamp.timestamp).to.equal(100);
		expect(newTimestamp.timestamp).to.equal(100 + (60 * 60 * 5 * 50));
	});

	it('supports toString', () => {
		// Arrange:
		const timestamp = new ConcreteNetworkTimestamp(123);

		// Act + Assert:
		expect(timestamp.toString()).to.equal('123');
	});
});

describe('NetworkTimestampDatetimeConverter', () => {
	const createConverter = () => new NetworkTimestampDatetimeConverter(new Date(Date.UTC(2020, 1, 2, 3)), 'hours');

	it('can convert epochal timestamp to datetime', () => {
		// Arrange:
		const converter = createConverter();

		// Act:
		const utcTimestamp = converter.toDatetime(0);

		// Assert:
		expect(utcTimestamp.getTime()).to.equal(Date.UTC(2020, 1, 2, 3));
	});

	it('can convert non epochal timestamp to datetime', () => {
		// Arrange:
		const converter = createConverter();

		// Act:
		const utcTimestamp = converter.toDatetime(5);

		// Assert:
		expect(utcTimestamp.getTime()).to.equal(Date.UTC(2020, 1, 2, 3 + 5));
	});

	it('cannot convert datetime before epochal timestamp', () => {
		// Arrange:
		const converter = createConverter();

		// Assert:
		expect(() => { converter.toDifference(new Date(Date.UTC(2020, 1, 2, 2))); }).to.throw('timestamp cannot be before epoch');
	});

	it('cannot convert datetime to epochal timestamp', () => {
		// Arrange:
		const converter = createConverter();

		// Act:
		const rawTimestamp = converter.toDifference(new Date(Date.UTC(2020, 1, 2, 3)));

		// Assert:
		expect(rawTimestamp).to.equal(0);
	});

	it('cannot convert datetime to non epochal timestamp', () => {
		// Arrange:
		const converter = createConverter();

		// Act:
		const rawTimestamp = converter.toDifference(new Date(Date.UTC(2020, 1, 2, 3 + 5)));

		// Assert:
		expect(rawTimestamp).to.equal(5);
	});
});
