const { Network, NetworkLocator } = require('../src/Network');
const { expect } = require('chai');

describe('Network', () => {
	it('supports toString', () => {
		// Arrange:
		const network = new Network('foo', 0x55);

		// Act + Assert:
		expect(network.toString()).to.equal('foo');
	});
});

const PREDEFINED_NETWORKS = [new Network('foo', 0x55), new Network('bar', 0x37)];

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
