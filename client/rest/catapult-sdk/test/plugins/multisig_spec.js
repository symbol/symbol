import { expect } from 'chai';
import multisig from '../../src/plugins/multisig';
import EntityType from '../../src/model/EntityType';
import ModelSchemaBuilder from '../../src/model/ModelSchemaBuilder';
import ModelType from '../../src/model/ModelType';
import test from '../binaryTestUtils';

const constants = {
	sizes: {
		modifyMultisigAccount: 3,
		modifications: 3 * (1 + 32)
	}
};

describe('multisig plugin', () => {
	describe('register schema', () => {
		it('adds multisig system schema', () => {
			// Arrange:
			const builder = new ModelSchemaBuilder();
			const numDefaultKeys = Object.keys(builder.build()).length;

			// Act:
			multisig.registerSchema(builder);
			const modelSchema = builder.build();

			// Assert:
			expect(Object.keys(modelSchema).length).to.equal(numDefaultKeys + 4);
			expect(modelSchema).to.contain.all.keys(['modifyMultisigAccount', 'modifyMultisigAccount.modification', 'multisigEntry']);

			// - modify multisig account
			expect(Object.keys(modelSchema.modifyMultisigAccount).length).to.equal(Object.keys(modelSchema.transaction).length + 1);
			expect(modelSchema.modifyMultisigAccount).to.contain.all.keys(['modifications']);

			// - cosignatory modification
			expect(modelSchema['modifyMultisigAccount.modification']).to.deep.equal({
				cosignatoryPublicKey: ModelType.binary
			});

			// - multisig entry
			expect(Object.keys(modelSchema.multisigEntry).length).to.equal(1);
			expect(modelSchema.multisigEntry).to.contain.all.keys(['multisig']);

			expect(Object.keys(modelSchema['multisigEntry.multisig']).length).to.equal(3);
			expect(modelSchema['multisigEntry.multisig']).to.contain.all.keys(['account', 'multisigAccounts', 'cosignatories']);
		});
	});

	describe('register codecs', () => {
		function getCodecs() {
			const codecs = {};
			multisig.registerCodecs({
				addTransactionSupport: (type, codec) => { codecs[type] = codec; }
			});

			return codecs;
		}

		it('adds multisig codec', () => {
			// Act:
			const codecs = getCodecs();

			// Assert: codec was registered
			expect(Object.keys(codecs).length).to.equal(1);
			expect(codecs).to.contain.all.keys([EntityType.modifyMultisigAccount.toString()]);
		});

		function generateTransaction() {
			return {
				buffer: Buffer.of(0xAB, 0xCD, 0x00),
				object: {
					minRemovalDelta: 0xAB,
					minApprovalDelta: 0xCD
				}
			};
		}

		function addModifications(generator) {
			const key1 = Buffer.of(
				0x77, 0xBE, 0xE1, 0xCA, 0xD0, 0x8E, 0x6E, 0x48, 0x95, 0xE8, 0x18, 0xB2, 0x7B, 0xD8, 0xFA, 0xC9,
				0x47, 0x0D, 0xB8, 0xFD, 0x2D, 0x81, 0x47, 0x6A, 0xC5, 0x61, 0xA4, 0xCE, 0xE1, 0x81, 0x40, 0x83);
			const key2 = Buffer.of(
				0x3E, 0xCA, 0x9E, 0x17, 0x1A, 0x02, 0xFB, 0xD4, 0x9C, 0x73, 0x75, 0x5D, 0x82, 0xEE, 0xCE, 0x6F,
				0x63, 0x90, 0x5A, 0x44, 0xA2, 0x7C, 0xF1, 0x3A, 0x7B, 0x77, 0xA9, 0xB3, 0x8A, 0xD1, 0xB2, 0x92);
			const key3 = Buffer.of(
				0x99, 0xF2, 0x26, 0x6C, 0x06, 0xBE, 0xE0, 0xE1, 0xC7, 0x39, 0x57, 0xFE, 0x0F, 0x39, 0x7E, 0x7A,
				0xE3, 0x15, 0xEA, 0x51, 0x6B, 0xA7, 0x12, 0xEF, 0x82, 0x7C, 0xE6, 0x2B, 0xD9, 0x5E, 0x01, 0xEC);

			return () => {
				const data = generator();
				data.buffer = Buffer.concat([
					data.buffer,
					Buffer.of(0x31),
					key1,
					Buffer.of(0x20),
					key2,
					Buffer.of(0x86),
					key3
				]);
				data.buffer.writeUInt8(3, constants.sizes.modifyMultisigAccount - 1);

				data.object.modifications = [
					{ modificationType: 0x31, cosignatoryPublicKey: key1 },
					{ modificationType: 0x20, cosignatoryPublicKey: key2 },
					{ modificationType: 0x86, cosignatoryPublicKey: key3 }
				];
				return data;
			};
		}

		function getCodec() {
			return getCodecs()[EntityType.modifyMultisigAccount];
		}

		describe('supports modify multisig account', () => {
			describe('with no modifications', () => {
				test.binary.test.addAll(getCodec(), constants.sizes.modifyMultisigAccount, generateTransaction);
			});

			describe('with modifications', () => {
				test.binary.test.addAll(
					getCodec(),
					constants.sizes.modifyMultisigAccount + constants.sizes.modifications,
					addModifications(generateTransaction));
			});
		});
	});
});
