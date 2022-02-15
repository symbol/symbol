const { Hash256, PublicKey } = require('../../src/CryptoTypes');
const { Address, Network } = require('../../src/symbol/Network');
const { hexToUint8 } = require('../../src/utils/converter');
const { runBasicAddressTests } = require('../test/addressTests');
const { runBasicNetworkTests } = require('../test/networkTests');
const { expect } = require('chai');

const MAINNET_GENERATION_HASH_SEED = new Hash256('57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6');
const TESTNET_GENERATION_HASH_SEED = new Hash256('7FCCD304802016BEBBCD342A332F91FF1F3BB5E902988B352697BE245F48E836');

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
		testnetNetwork: Network.TESTNET
	});

	it('registers correct predefined networks', () => {
		expect(Network.NETWORKS.length).to.equal(2);
		expect(Network.NETWORKS.map(network => network.name)).to.deep.equal(['mainnet', 'testnet']);

		expect(Network.MAINNET.name).to.equal('mainnet');
		expect(Network.MAINNET.identifier).to.equal(0x68);
		expect(Network.MAINNET.generationHashSeed).to.deep.equal(MAINNET_GENERATION_HASH_SEED);

		expect(Network.TESTNET.name).to.equal('testnet');
		expect(Network.TESTNET.identifier).to.equal(0x98);
		expect(Network.TESTNET.generationHashSeed).to.deep.equal(TESTNET_GENERATION_HASH_SEED);
	});
});
