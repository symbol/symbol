import embeddedEntityCodec from '../../src/modelBinary/embeddedEntityCodec';
import test from '../binaryTestUtils';

describe('embedded entity codec', () => {
	function generateEmbeddedEntity() {
		const Signer_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.signer));

		return {
			buffer: Buffer.concat([
				Signer_Buffer,
				Buffer.of(0x2A, 0x81, 0x1C, 0x45) // version, type
			]),
			object: {
				signer: Signer_Buffer,
				version: 0x812A,
				type: 0x451C
			}
		};
	}

	test.binary.test.addAll(embeddedEntityCodec, 36, generateEmbeddedEntity);
});
