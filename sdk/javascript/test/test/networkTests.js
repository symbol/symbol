import { NetworkTimestamp } from '../../src/NetworkTimestamp.js';
import { expect } from 'chai';

export const runBasicNetworkTests = testDescriptor => { // eslint-disable-line import/prefer-default-export
	const getTimeUnits = value => value * testDescriptor.timeUnitMultiplier;

	// region publicKeyToAddress

	it('can convert mainnet public key to address', () => {
		// Act:
		const address = testDescriptor.mainnetNetwork.publicKeyToAddress(testDescriptor.deterministicPublicKey);

		// Assert:
		expect(address).to.deep.equal(testDescriptor.expectedMainnetAddress);
	});

	it('can convert testnet public key to address', () => {
		// Act:
		const address = testDescriptor.testnetNetwork.publicKeyToAddress(testDescriptor.deterministicPublicKey);

		// Assert:
		expect(address).to.deep.equal(testDescriptor.expectedTestnetAddress);
	});

	// endregion

	// region isValidAddress[String]

	const canValidateValidateAddress = network => {
		// Arrange:
		const address = network.publicKeyToAddress(testDescriptor.deterministicPublicKey);

		// Act + Assert:
		expect(network.isValidAddress(address)).to.equal(true);
		expect(network.isValidAddressString(address.toString())).to.equal(true);
	};

	const mutateBytes = (bytes, signedPosition) => {
		const position = 0 > signedPosition ? bytes.length + signedPosition : signedPosition;
		return new Uint8Array([...bytes.subarray(0, position), bytes[position] ^ 0xFF, ...bytes.subarray(position + 1)]);
	};

	const cannotValidateInvalidAddress = (network, position) => {
		// Arrange:
		const address = network.publicKeyToAddress(testDescriptor.deterministicPublicKey);
		address.bytes = mutateBytes(address.bytes, position);

		// Act + Assert:
		expect(network.isValidAddress(address)).to.equal(false);
		expect(network.isValidAddressString(address.toString())).to.equal(false);
	};

	const cannotValidateInvalidAddressString = (network, mutator) => {
		// Arrange:
		const address = network.publicKeyToAddress(testDescriptor.deterministicPublicKey);
		const addressString = mutator(address.toString());

		// Act + Assert:
		expect(network.isValidAddressString(addressString)).to.equal(false);
	};

	const addNetworkTests = (network, networkName) => {
		it(`can convert ${networkName} public key to address`, () => {
			// Act:
			const address = network.publicKeyToAddress(testDescriptor.deterministicPublicKey);

			// Assert:
			expect(address).to.deep.equal(testDescriptor[`expected${networkName[0].toUpperCase()}${networkName.substring(1)}Address`]);
		});

		it(`can validate valid ${networkName} address`, () => {
			canValidateValidateAddress(network);
		});

		it(`cannot validate invalid ${networkName} address (begin)`, () => {
			cannotValidateInvalidAddress(network, 1);
		});

		it(`cannot validate invalid ${networkName} address (end)`, () => {
			cannotValidateInvalidAddress(network, -1);
		});

		it(`cannot validate invalid ${networkName} address string (invalid size)`, () => {
			cannotValidateInvalidAddressString(network, addressString => `${addressString}A`);
			cannotValidateInvalidAddressString(network, addressString => addressString.substring(0, addressString.length - 1));
		});

		it(`cannot validate invalid ${networkName} address string (invalid char)`, () => {
			cannotValidateInvalidAddressString(
				network,
				addressString => `${addressString.substring(0, 10)}@${addressString.substring(11)}`
			);
		});
	};

	addNetworkTests(testDescriptor.mainnetNetwork, 'mainnet');
	addNetworkTests(testDescriptor.testnetNetwork, 'testnet');

	// endregion

	// region toDatetime

	it('can convert epochal timestamp to datetime', () => {
		// Arrange:
		const network = testDescriptor.mainnetNetwork;
		const { epoch } = network.datetimeConverter;

		// Act:
		const datetimeTimestamp = network.toDatetime(new NetworkTimestamp(0));

		// Assert:
		expect(datetimeTimestamp.getTime()).to.equal(epoch.getTime());
	});

	it('can convert non epochal timestamp to datetime', () => {
		// Arrange:
		const network = testDescriptor.mainnetNetwork;
		const { epoch } = network.datetimeConverter;

		// Act:
		const datetimeTimestamp = network.toDatetime(new NetworkTimestamp(123));

		// Assert:
		expect(datetimeTimestamp.getTime()).to.equal(epoch.getTime() + getTimeUnits(123));
	});

	// endregion

	// region fromDatetime

	it('can convert datetime to epochal timestamp', () => {
		// Arrange:
		const network = testDescriptor.mainnetNetwork;
		const { epoch } = network.datetimeConverter;

		// Act:
		const networkTimestamp = network.fromDatetime(epoch);

		// Assert:
		expect(networkTimestamp.isEpochal).to.equal(true);
		expect(networkTimestamp.timestamp).to.equal(0n);
	});

	it('can convert datetime to non epochal timestamp', () => {
		// Arrange:
		const network = testDescriptor.mainnetNetwork;
		const { epoch } = network.datetimeConverter;

		// Act:
		const networkTimestamp = network.fromDatetime(new Date(epoch.getTime() + getTimeUnits(123)));

		// Assert:
		expect(networkTimestamp.isEpochal).to.equal(false);
		expect(networkTimestamp.timestamp).to.equal(123n);
	});

	// endregion
};
