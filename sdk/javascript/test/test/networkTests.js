const { expect } = require('chai');

const runBasicNetworkTests = testDescriptor => {
	const canValidateValidateAddress = network => {
		// Arrange:
		const address = network.publicKeyToAddress(testDescriptor.deterministicPublicKey);

		// Act + Assert:
		expect(network.isValidAddress(address)).to.equal(true);
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
	};

	const addNetworkTests = (network, networkName) => {
		it(`can convert ${networkName} public key to address`, () => {
			// Act:
			const address = testDescriptor.mainnetNetwork.publicKeyToAddress(testDescriptor.deterministicPublicKey);

			// Assert:
			expect(address).to.deep.equal(testDescriptor.expectedMainnetAddress);
		});

		it(`can validate valid ${networkName} address`, () => {
			canValidateValidateAddress(testDescriptor.mainnetNetwork);
		});

		it(`cannot validate invalid ${networkName} address (begin)`, () => {
			cannotValidateInvalidAddress(testDescriptor.mainnetNetwork, 1);
		});

		it(`can validate invalid ${networkName} address (end)`, () => {
			cannotValidateInvalidAddress(testDescriptor.mainnetNetwork, -1);
		});
	};

	addNetworkTests(testDescriptor.mainnetNetwork, 'mainnet');
	addNetworkTests(testDescriptor.testnetNetwork, 'testnet');
};

module.exports = { runBasicNetworkTests };
