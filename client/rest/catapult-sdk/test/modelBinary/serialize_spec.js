import { expect } from 'chai';
import serialize from '../../src/modelBinary/serialize';
import convert from '../../src/utils/convert';

describe('serialize', () => {
	function getCodec() {
		return {
			deserialize: parser => {
				const transaction = {};
				transaction.alpha = parser.uint16();
				transaction.beta = parser.uint32();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint16(transaction.alpha);
				serializer.writeUint32(transaction.beta);
			}
		};
	}

	function runTest(actAndAssert) {
		// Arrange:
		const expectedBuffer = Buffer.concat([
			Buffer.of(0x46, 0x8B), // alpha
			Buffer.of(0xFE, 0x30, 0xE8, 0x50) // beta
		]);

		const transaction = {
			alpha: 0x8B46,
			beta: 0x50E830FE
		};

		// Act + Assert:
		actAndAssert(expectedBuffer, transaction);
	}

	it('to buffer returns appropriate buffer', () => {
		// Arrange:
		runTest((expectedBuffer, transaction) => {
			// Act:
			const buffer = serialize.toBuffer(getCodec(), transaction);

			// Assert:
			expect(buffer).to.deep.equal(expectedBuffer);
		});
	});

	it('to hex returns appropriate hex string', () => {
		// Arrange:
		runTest((expectedBuffer, transaction) => {
			const expectedHex = convert.uint8ToHex(expectedBuffer);

			// Act:
			const hex = serialize.toHex(getCodec(), transaction);

			// Assert:
			expect(hex).to.equal(expectedHex);
		});
	});
});
