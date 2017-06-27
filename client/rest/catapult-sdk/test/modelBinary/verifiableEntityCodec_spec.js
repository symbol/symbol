import verifiableEntityCodec from '../../src/modelBinary/verifiableEntityCodec';
import test from '../binaryTestUtils';

describe('verifiable entity codec', () => {
	function generateVerifiableEntity() {
		const Signature_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signature));
		const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signer));

		return {
			buffer: Buffer.concat([
				Signature_Buffer,
				Signer_Buffer,
				Buffer.of(0x2A, 0x81, 0x1C, 0x45) // version, type
			]),
			object: {
				signature: Signature_Buffer,
				signer: Signer_Buffer,
				version: 0x812A,
				type: 0x451C
			}
		};
	}

	test.binary.test.addAll(verifiableEntityCodec, 100, generateVerifiableEntity);
});
