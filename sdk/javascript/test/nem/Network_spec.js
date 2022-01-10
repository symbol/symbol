const { PublicKey } = require('../../src/CryptoTypes');
const { Address, Network } = require('../../src/nem/Network');
const { hexToUint8 } = require('../../src/utils/converter');
const { runBasicAddressTests } = require('../test/addressTests');
const { runBasicNetworkTests } = require('../test/networkTests');
const { expect } = require('chai');

describe('Address (NEM)', () => {
	runBasicAddressTests({
		Address,
		encodedAddress: 'TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33',
		decodedAddress: hexToUint8('988A692D13959918BA9A33BE57B114A0C8BA3E59A3684C977B')
	});
});

describe('Network (NEM)', () => {
	runBasicNetworkTests({
		Network,
		deterministicPublicKey: new PublicKey('D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0'),
		expectedMainnetAddress: new Address('NCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNJABUMH'),
		mainnetNetwork: Network.MAINNET,
		expectedTestnetAddress: new Address('TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33'),
		testnetNetwork: Network.TESTNET
	});

	it('registers correct predefined networks', () => {
		expect(Network.NETWORKS.length).to.equal(2);
		expect(Network.NETWORKS.map(network => network.name)).to.deep.equal(['mainnet', 'testnet']);

		expect(Network.MAINNET.name).to.equal('mainnet');
		expect(Network.MAINNET.identifier).to.equal(0x68);

		expect(Network.TESTNET.name).to.equal('testnet');
		expect(Network.TESTNET.identifier).to.equal(0x98);
	});
});
