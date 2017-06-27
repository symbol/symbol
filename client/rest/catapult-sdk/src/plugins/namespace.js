/** @module plugins/namespace */
import EntityType from '../model/EntityType';
import ModelType from '../model/ModelType';

function isNamespaceTypeRoot(namespaceType) {
	return 0 === namespaceType;
}

function parseString(parser, size) {
	return parser.buffer(size).toString('ascii');
}

function writeString(serializer, str) {
	serializer.writeBuffer(Buffer.from(str, 'ascii'));
}

/**
 * Creates a namespace plugin.
 * @type {module:plugins/CatapultPlugin}
 */
export default {
	registerSchema: builder => {
		builder.addTransactionSupport('registerNamespace', {
			namespaceId: ModelType.uint64,
			parentId: ModelType.uint64,
			duration: ModelType.uint64,
			name: ModelType.string
		});

		builder.addTransactionSupport('mosaicDefinition', {
			mosaicId: ModelType.uint64,
			parentId: ModelType.uint64,
			name: ModelType.string,
			properties: { type: ModelType.array, schemaName: 'mosaicDefinition.mosaicProperty' }
		});
		builder.addSchema('mosaicDefinition.mosaicProperty', {
			value: ModelType.uint64
		});

		builder.addTransactionSupport('mosaicSupplyChange', {
			mosaicId: ModelType.uint64,
			delta: ModelType.uint64
		});

		builder.addSchema('mosaicDescriptor', {
			meta: { type: ModelType.object, schemaName: 'transactionMetadata' },
			mosaic: { type: ModelType.object, schemaName: 'mosaicDescriptor.mosaic' }
		});
		builder.addSchema('mosaicDescriptor.mosaic', {
			namespaceId: ModelType.uint64,
			mosaicId: ModelType.uint64,
			supply: ModelType.uint64,

			height: ModelType.uint64,
			owner: ModelType.binary,
			properties: { type: ModelType.array, schemaName: ModelType.uint64 }
		});

		builder.addSchema('namespaceDescriptor', {
			meta: { type: ModelType.object, schemaName: 'transactionMetadata' },
			namespace: { type: ModelType.object, schemaName: 'namespaceDescriptor.namespace' }
		});
		builder.addSchema('namespaceDescriptor.namespace', {
			level0: ModelType.uint64,
			level1: ModelType.uint64,
			level2: ModelType.uint64,

			parentId: ModelType.uint64,
			owner: ModelType.binary,

			startHeight: ModelType.uint64,
			endHeight: ModelType.uint64
		});

		builder.addSchema('mosaicNameTuple', {
			mosaicId: ModelType.uint64,
			name: ModelType.string,
			parentId: ModelType.uint64
		});
		builder.addSchema('namespaceNameTuple', {
			namespaceId: ModelType.uint64,
			name: ModelType.string,
			parentId: ModelType.uint64
		});
	},

	registerCodecs: codecBuilder => {
		codecBuilder.addTransactionSupport(EntityType.registerNamespace, {
			deserialize: parser => {
				const transaction = {};
				transaction.namespaceType = parser.uint8();
				transaction[isNamespaceTypeRoot(transaction.namespaceType) ? 'duration' : 'parentId'] = parser.uint64();
				transaction.namespaceId = parser.uint64();

				const namespaceNameSize = parser.uint8();
				transaction.namespaceName = parseString(parser, namespaceNameSize);
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint8(transaction.namespaceType);
				serializer.writeUint64(isNamespaceTypeRoot(transaction.namespaceType) ? transaction.duration : transaction.parentId);
				serializer.writeUint64(transaction.namespaceId);

				serializer.writeUint8(transaction.namespaceName.length);
				writeString(serializer, transaction.namespaceName);
			}
		});

		codecBuilder.addTransactionSupport(EntityType.mosaicDefinition, {
			deserialize: parser => {
				const transaction = {};
				transaction.parentId = parser.uint64();
				transaction.mosaicId = parser.uint64();

				const mosaicNameSize = parser.uint8();
				const propertiesCount = parser.uint8();

				transaction.flags = parser.uint8();
				transaction.divisibility = parser.uint8();

				transaction.mosaicName = parseString(parser, mosaicNameSize);

				if (0 < propertiesCount) {
					transaction.properties = [];
					for (let i = 0; i < propertiesCount; ++i) {
						const key = parser.uint8();
						const value = parser.uint64();
						transaction.properties.push({ key, value });
					}
				}

				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.parentId);
				serializer.writeUint64(transaction.mosaicId);

				serializer.writeUint8(transaction.mosaicName.length);
				const propertiesCount = undefined === transaction.properties ? 0 : transaction.properties.length;
				serializer.writeUint8(propertiesCount);

				serializer.writeUint8(transaction.flags);
				serializer.writeUint8(transaction.divisibility);

				writeString(serializer, transaction.mosaicName);
				for (let i = 0; i < propertiesCount; ++i) {
					const property = transaction.properties[i];
					serializer.writeUint8(property.key);
					serializer.writeUint64(property.value);
				}
			}
		});

		codecBuilder.addTransactionSupport(EntityType.mosaicSupplyChange, {
			deserialize: parser => {
				const transaction = {};
				transaction.mosaicId = parser.uint64();
				transaction.direction = parser.uint8();
				transaction.delta = parser.uint64();
				return transaction;
			},

			serialize: (transaction, serializer) => {
				serializer.writeUint64(transaction.mosaicId);
				serializer.writeUint8(transaction.direction);
				serializer.writeUint64(transaction.delta);
			}
		});
	}
};
