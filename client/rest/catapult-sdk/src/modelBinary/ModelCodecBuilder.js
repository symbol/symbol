/** @module modelBinary/ModelCodecBuilder */
import blockHeaderCodec from './blockHeaderCodec';
import transactionCodec from './transactionCodec';
import verifiableEntityCodec from './verifiableEntityCodec';
import SerializedSizeCalculator from '../serializer/SerializedSizeCalculator';

const constants = {
	sizes: {
		blockHeader: 192,
		transactionHeader: 120
	}
};

function isBlockType(entityType) {
	return 0 !== (0x8000 & entityType);
}

function findCodecs(entityType, codecs) {
	if (isBlockType(entityType))
		return [verifiableEntityCodec, blockHeaderCodec];

	const codec = codecs[entityType];
	if (!codec)
		throw Error(`no codec registered for '${entityType}'`);

	return [verifiableEntityCodec, transactionCodec, codec];
}

function createThrowawayConsumingCodec(size) {
	return {
		deserialize: parser => { parser.buffer(size); }
	};
}

function deserializeTransactions(parser, size, txCodecs) {
	const transactions = [];
	let remainingBytes = size;
	while (0 < remainingBytes) {
		const transactionSize = parser.uint32();
		if (transactionSize < constants.sizes.transactionHeader)
			throw Error('transaction must contain complete transaction header');

		remainingBytes -= transactionSize;
		const entity = verifiableEntityCodec.deserialize(parser);

		// allow unknown txes to be deserialized as basic txes
		const codecs = [transactionCodec];
		if (txCodecs[entity.type])
			codecs.push(txCodecs[entity.type]);
		else
			codecs.push(createThrowawayConsumingCodec(transactionSize - constants.sizes.transactionHeader));

		for (const codec of codecs)
			Object.assign(entity, codec.deserialize(parser, transactionSize, txCodecs));

		transactions.push(entity);
	}

	return transactions;
}

/**
 * Builder for creating an aggregate model codec.
 */
export default class ModelCodecBuilder {
	/**
	 * Creates a model codec builder.
	 */
	constructor() {
		this.codecs = [];
	}

	/**
	 * Adds support for a typed transaction.
	 * @param {module:model/EntityType} type The transaction type.
	 * @param {object} codec The transaction codec.
	 */
	addTransactionSupport(type, codec) {
		if (isBlockType(type) || this.codecs[type])
			throw Error(`codec already registered for '${type}'`);

		this.codecs[type] = codec;
	}

	/**
	 * Builds and returns an appropriate aggregate model codec.
	 * @returns {module:modelBinary/AggregateModelCodec} The aggregate model codec.
	 */
	build() {
		const txCodecs = this.codecs;
		return {
			supports(type) {
				return isBlockType(type) || undefined !== txCodecs[type];
			},

			deserialize: parser => {
				// get codecs for the current entity (and ignore the verifiableEntity codec)
				const size = parser.uint32();
				const entity = verifiableEntityCodec.deserialize(parser);
				const codecs = findCodecs(entity.type, txCodecs);
				codecs.shift();

				for (const codec of codecs)
					Object.assign(entity, codec.deserialize(parser, size, txCodecs));

				// if it's a block with transactions, also deserialize them
				if (isBlockType(entity.type) && constants.sizes.blockHeader !== size) {
					const extraSize = size - constants.sizes.blockHeader;
					if (0 > extraSize)
						throw Error('block must contain complete block header');

					entity.transactions = deserializeTransactions(parser, extraSize, txCodecs);
				}

				return entity;
			},

			serialize: (entity, serializer) => {
				const codecs = findCodecs(entity.type, txCodecs);

				const sizeCalculator = new SerializedSizeCalculator();
				for (const codec of codecs)
					codec.serialize(entity, sizeCalculator, txCodecs);

				serializer.writeUint32(sizeCalculator.size() + 4); // include size of size field itself
				for (const codec of codecs)
					codec.serialize(entity, serializer, txCodecs);
			}
		};
	}
}
