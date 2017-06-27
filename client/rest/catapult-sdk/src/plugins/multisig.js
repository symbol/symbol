/** @module plugins/multisig */
import EntityType from '../model/EntityType';
import ModelType from '../model/ModelType';
import sizes from '../modelBinary/sizes';

const constants = { sizes };

/**
 * Creates a multisig plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport('modifyMultisigAccount', {
			modifications: { type: ModelType.array, schemaName: 'modifyMultisigAccount.modification' }
		});
		builder.addSchema('modifyMultisigAccount.modification', {
			cosignatoryPublicKey: ModelType.binary
		});

		builder.addSchema('multisigEntry', {
			multisig: { type: ModelType.object, schemaName: 'multisigEntry.multisig' }
		});
		builder.addSchema('multisigEntry.multisig', {
			account: ModelType.binary,
			multisigAccounts: { type: ModelType.array, schemaName: ModelType.binary },
			cosignatories: { type: ModelType.array, schemaName: ModelType.binary }
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.modifyMultisigAccount, {
			deserialize: parser => {
				const transaction = {};
				transaction.minRemovalDelta = parser.uint8();
				transaction.minApprovalDelta = parser.uint8();

				const numModifications = parser.uint8();
				if (0 < numModifications) {
					transaction.modifications = [];
					while (transaction.modifications.length < numModifications) {
						const modificationType = parser.uint8();
						const cosignatoryPublicKey = parser.buffer(constants.sizes.signer);
						transaction.modifications.push({ modificationType, cosignatoryPublicKey });
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.minRemovalDelta);
				serializer.writeUint8(transaction.minApprovalDelta);

				const numModifications = transaction.modifications ? transaction.modifications.length : 0;
				serializer.writeUint8(numModifications);

				if (0 < numModifications) {
					for (const modification of transaction.modifications) {
						serializer.writeUint8(modification.modificationType);
						serializer.writeBuffer(modification.cosignatoryPublicKey);
					}
				}
			}
		});
	}
};
