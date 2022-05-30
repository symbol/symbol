const { Hash256, PublicKey } = require('../../src/CryptoTypes');
const { Address, Network, NetworkTimestamp } = require('../../src/symbol/Network');
const { hexToUint8 } = require('../../src/utils/converter');
const { runBasicAddressTests } = require('../test/addressTests');
const { runBasicNetworkTests } = require('../test/networkTests');
const { expect } = require('chai');

const MAINNET_GENERATION_HASH_SEED = new Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6');
const TESTNET_GENERATION_HASH_SEED = new Hash256('7FCCD304802016BEBBCD342A332F91FF1F3BB5E902988B352697BE245F48E836');

describe('NetworkTimestamp (Symbol)', () => {
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

describe('Address (Symbol)', () => {
	runBasicAddressTests({
		Address,
		encodedAddress: 'TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA',
		decodedAddress: hexToUint8('985783F7A83BE5D8084C6DA3B366CAAFACC129F36A3EA90C')
	});
});

describe('Network (Symbol)', () => {
	runBasicNetworkTests({
		Network,
		deterministicPublicKey: new PublicKey('C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42F68C7CA1772FE8A0'),
		expectedMainnetAddress: new Address('NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY'),
		mainnetNetwork: Network.MAINNET,
		expectedTestnetAddress: new Address('TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA'),
		testnetNetwork: Network.TESTNET,
		timeUnitMultiplier: 1
	});

	it('registers correct predefined networks', () => {
		expect(Network.NETWORKS.length).to.equal(2);
		expect(Network.NETWORKS.map(network => network.name)).to.deep.equal(['mainnet', 'testnet']);

		expect(Network.MAINNET.name).to.equal('mainnet');
		expect(Network.MAINNET.identifier).to.equal(0x68);
		expect(Network.MAINNET.datetimeConverter.epoch.toUTCString()).to.equal('Tue, 16 Mar 2021 00:06:25 GMT');
		expect(Network.MAINNET.datetimeConverter.timeUnits).to.equal(1);
		expect(Network.MAINNET.generationHashSeed).to.deep.equal(MAINNET_GENERATION_HASH_SEED);

		expect(Network.TESTNET.name).to.equal('testnet');
		expect(Network.TESTNET.identifier).to.equal(0x98);
		expect(Network.TESTNET.datetimeConverter.epoch.toUTCString()).to.equal('Thu, 25 Nov 2021 14:00:47 GMT');
		expect(Network.TESTNET.datetimeConverter.timeUnits).to.equal(1);
		expect(Network.TESTNET.generationHashSeed).to.deep.equal(TESTNET_GENERATION_HASH_SEED);
	});
});
