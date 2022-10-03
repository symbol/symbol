import { NetworkTimestamp, NetworkTimestampDatetimeConverter } from '../src/NetworkTimestamp.js';
import { expect } from 'chai';

describe('NetworkTimestamp', () => {
	const runTestCases = wrapInt => {
		class ConcreteNetworkTimestamp extends NetworkTimestamp { // needed because NetworkTimestamp is abstract
			addSeconds(count) {
				return new ConcreteNetworkTimestamp(this.timestamp + (5n * BigInt(count)));
			}
		}

		it('can create epochal timestamp', () => {
			// Act:
			const timestamp = new ConcreteNetworkTimestamp(wrapInt(0));

			// Assert:
			expect(timestamp.isEpochal).to.equal(true);
			expect(timestamp.timestamp).to.equal(0n);
		});

		it('can create non epochal timestamp', () => {
			// Act:
			const timestamp = new ConcreteNetworkTimestamp(wrapInt(123));

			// Assert:
			expect(timestamp.isEpochal).to.equal(false);
			expect(timestamp.timestamp).to.equal(123n);
		});

		it('can add minutes', () => {
			// Arrange:
			const timestamp = new ConcreteNetworkTimestamp(wrapInt(100));

			// Act:
			const newTimestamp = timestamp.addMinutes(wrapInt(50));

			// Assert:
			expect(timestamp.timestamp).to.equal(100n);
			expect(newTimestamp.timestamp).to.equal(100n + (60n * 5n * 50n));
		});

		it('can add hours', () => {
			// Arrange:
			const timestamp = new ConcreteNetworkTimestamp(wrapInt(100));

			// Act:
			const newTimestamp = timestamp.addHours(wrapInt(50));

			// Assert:
			expect(timestamp.timestamp).to.equal(100n);
			expect(newTimestamp.timestamp).to.equal(100n + (60n * 60n * 5n * 50n));
		});

		it('supports toString', () => {
			// Arrange:
			const timestamp = new ConcreteNetworkTimestamp(wrapInt(123));

			// Act + Assert:
			expect(timestamp.toString()).to.equal('123');
		});
	};

	describe('works with Number', () => {
		runTestCases(n => n);
	});

	describe('works with BigInt', () => {
		runTestCases(n => BigInt(n));
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
