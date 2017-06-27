import blockHeaderCodec from '../../src/modelBinary/blockHeaderCodec';
import test from '../binaryTestUtils';

describe('block header codec', () => {
	function generateBlockHeader() {
		const Previous_Block_Hash_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash));
		const Block_Transactions_Hash_Buffer = Buffer.from(test.random.bytes(test.constants.sizes.hash));

		return {
			buffer: Buffer.concat([
				Buffer.of(0x97, 0x87, 0x45, 0x0E, 0xE1, 0x6C, 0xB6, 0x62), // height
				Buffer.of(0x30, 0x3A, 0x46, 0x8B, 0x15, 0x2D, 0x60, 0x54), // timestamp
				Buffer.of(0x86, 0x02, 0x75, 0x30, 0xE8, 0x50, 0x78, 0xE8), // difficulty
				Previous_Block_Hash_Buffer,
				Block_Transactions_Hash_Buffer
			]),
			object: {
				height: [0x0E458797, 0x62B66CE1],
				timestamp: [0x8B463A30, 0x54602D15],
				difficulty: [0x30750286, 0xE87850E8],
				previousBlockHash: Previous_Block_Hash_Buffer,
				blockTransactionsHash: Block_Transactions_Hash_Buffer
			}
		};
	}

	test.binary.test.addAll(blockHeaderCodec, 88, generateBlockHeader);
});
