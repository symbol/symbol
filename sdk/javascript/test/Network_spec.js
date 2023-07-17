import { Network, NetworkLocator } from '../src/Network.js';
import { NetworkTimestamp, NetworkTimestampDatetimeConverter } from '../src/NetworkTimestamp.js';
import { expect } from 'chai';

const createNetwork = (name, identifier) => {
	class MockAddress {
		static ENCODED_SIZE = 0;
	}

	const converter = new NetworkTimestampDatetimeConverter(new Date(Date.UTC(2022, 2, 16, 0, 6, 25)), 'minutes');
	return new Network(name, identifier, converter, () => {}, () => {}, MockAddress, NetworkTimestamp);
};

describe('Network', () => {
	it('can convert network time to datetime', () => {
		// Arrange:
		const network = createNetwork('foo', 0x55);

		// Act:
		const datetimeTimestamp = network.toDatetime(new NetworkTimestamp(60));

		// Assert:
		expect(datetimeTimestamp.getTime()).to.equal(new Date(Date.UTC(2022, 2, 16, 1, 6, 25)).getTime());
	});

	it('can convert datetime to network time', () => {
		// Arrange:
		const network = createNetwork('foo', 0x55);

		// Act:
		const networkTimestamp = network.fromDatetime(new Date(Date.UTC(2022, 2, 16, 1, 6, 25)));

		// Assert:
		expect(networkTimestamp.timestamp).to.equal(60n);
	});

	it('supports toString', () => {
		// Arrange:
		const network = createNetwork('foo', 0x55);

		// Act + Assert:
		expect(network.toString()).to.equal('foo');
	});
});

const PREDEFINED_NETWORKS = [createNetwork('foo', 0x55), createNetwork('bar', 0x37)];

describe('NetworkLocator', () => {
	it('can find well known network by name (single)', () => {
		expect(PREDEFINED_NETWORKS[0]).to.deep.equal(NetworkLocator.findByName(PREDEFINED_NETWORKS, 'foo'));
		expect(PREDEFINED_NETWORKS[1]).to.deep.equal(NetworkLocator.findByName(PREDEFINED_NETWORKS, 'bar'));
	});

	it('can find well known network by name (list)', () => {
		expect(PREDEFINED_NETWORKS[0]).to.deep.equal(NetworkLocator.findByName(PREDEFINED_NETWORKS, ['xxx', 'foo']));
		expect(PREDEFINED_NETWORKS[1]).to.deep.equal(NetworkLocator.findByName(PREDEFINED_NETWORKS, ['bar', 'yyy']));
		expect(PREDEFINED_NETWORKS[0]).to.deep.equal(NetworkLocator.findByName(PREDEFINED_NETWORKS, ['bar', 'foo']));
	});

	it('cannot find other network by name', () => {
		expect(() => { NetworkLocator.findByName(PREDEFINED_NETWORKS, 'cat'); }).to.throw(RangeError);
		expect(() => { NetworkLocator.findByName(PREDEFINED_NETWORKS, ['cat', 'dog']); }).to.throw(RangeError);
	});

	it('can find well known network by identifier (single)', () => {
		expect(PREDEFINED_NETWORKS[0]).to.deep.equal(NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, 0x55));
		expect(PREDEFINED_NETWORKS[1]).to.deep.equal(NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, 0x37));
	});

	it('can find well known network by identifier (list)', () => {
		expect(PREDEFINED_NETWORKS[0]).to.deep.equal(NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, [0x88, 0x55]));
		expect(PREDEFINED_NETWORKS[1]).to.deep.equal(NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, [0x37, 0x99]));
		expect(PREDEFINED_NETWORKS[0]).to.deep.equal(NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, [0x37, 0x55]));
	});

	it('cannot find other network by identifier', () => {
		expect(() => { NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, 0xFF); }).to.throw(RangeError);
		expect(() => { NetworkLocator.findByIdentifier(PREDEFINED_NETWORKS, [0xFF, 0x88]); }).to.throw(RangeError);
	});
});
