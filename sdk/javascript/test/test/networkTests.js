const { expect } = require('chai');

const runBasicNetworkTests = testDescriptor => {
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
};

module.exports = { runBasicNetworkTests };
