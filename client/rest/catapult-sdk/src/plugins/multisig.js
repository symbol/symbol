/** @module plugins/multisig */
const EntityType = require('../model/EntityType');
const ModelType = require('../model/ModelType');
const sizes = require('../modelBinary/sizes');
const convert = require('../utils/convert');

const constants = { sizes };

/**
 * Creates a multisig plugin.
 * @type {module:plugins/CatapultPlugin}
 */
module.exports = {
	registerSchema: builder => {
		builder.addTransactionSupport(EntityType.modifyMultisigAccount, {
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
			accountAddress: ModelType.binary,
			multisigAccounts: { type: ModelType.array, schemaName: ModelType.binary },
			cosignatories: { type: ModelType.array, schemaName: ModelType.binary }
		});
		builder.addSchema('multisigGraph', {
			multisigEntries: { type: ModelType.array, schemaName: 'multisigEntry' }
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.modifyMultisigAccount, {
			deserialize: parser => {
				const transaction = {};
				transaction.minRemovalDelta = convert.uint8ToInt8(parser.uint8());
				transaction.minApprovalDelta = convert.uint8ToInt8(parser.uint8());

				const numModifications = parser.uint8();
				if (0 < numModifications) {
					transaction.modifications = [];
					while (transaction.modifications.length < numModifications) {
						const type = parser.uint8();
						const cosignatoryPublicKey = parser.buffer(constants.sizes.signer);
						transaction.modifications.push({ type, cosignatoryPublicKey });
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(convert.int8ToUint8(transaction.minRemovalDelta));
				serializer.writeUint8(convert.int8ToUint8(transaction.minApprovalDelta));

				const numModifications = transaction.modifications ? transaction.modifications.length : 0;
				serializer.writeUint8(numModifications);

				if (0 < numModifications) {
					transaction.modifications.forEach(modification => {
						serializer.writeUint8(modification.type);
						serializer.writeBuffer(modification.cosignatoryPublicKey);
					});
				}
			}
		});
	}
};
