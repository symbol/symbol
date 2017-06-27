import transactionCodec from '../../src/modelBinary/transactionCodec';
import test from '../binaryTestUtils';

describe('transaction codec', () => {
	function generateTransaction() {
		return {
			buffer: Buffer.concat([
				Buffer.of(0x18, 0xA2, 0x46, 0xD0, 0x56, 0xDC, 0x18, 0xB0), // fee
				Buffer.of(0x4A, 0xE0, 0xDA, 0x7F, 0x93, 0x73, 0x11, 0xC0) // deadline
			]),
			object: {
				fee: [0xD046A218, 0xB018DC56],
				deadline: [0x7FDAE04A, 0xC0117393]
			}
		};
	}

	test.binary.test.addAll(transactionCodec, 16, generateTransaction);
});
