import { PublicKey } from '../../src/CryptoTypes.js';
import { Address, Network, NetworkTimestamp } from '../../src/nem/Network.js';
import { hexToUint8 } from '../../src/utils/converter.js';
import { runBasicAddressTests } from '../test/addressTests.js';
import { runBasicNetworkTests } from '../test/networkTests.js';
import { expect } from 'chai';

describe('NetworkTimestamp (NEM)', () => {
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
		testnetNetwork: Network.TESTNET,
		timeUnitMultiplier: 1000
	});

	it('registers correct predefined networks', () => {
		expect(Network.NETWORKS.length).to.equal(2);
		expect(Network.NETWORKS.map(network => network.name)).to.deep.equal(['mainnet', 'testnet']);

		expect(Network.MAINNET.name).to.equal('mainnet');
		expect(Network.MAINNET.identifier).to.equal(0x68);
		expect(Network.MAINNET.datetimeConverter.epoch.toUTCString()).to.equal('Sun, 29 Mar 2015 00:06:25 GMT');
		expect(Network.MAINNET.datetimeConverter.timeUnits).to.equal(1000);

		expect(Network.TESTNET.name).to.equal('testnet');
		expect(Network.TESTNET.identifier).to.equal(0x98);
		expect(Network.TESTNET.datetimeConverter.epoch.toUTCString()).to.equal('Sun, 29 Mar 2015 00:06:25 GMT');
		expect(Network.TESTNET.datetimeConverter.timeUnits).to.equal(1000);
	});
});
