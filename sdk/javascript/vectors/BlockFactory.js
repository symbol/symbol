import ByteArray from '../src/ByteArray.js';
import { Hash256, PublicKey } from '../src/CryptoTypes.js';
import RuleBasedTransactionFactory from '../src/RuleBasedTransactionFactory.js';
import {
	Address,
	/* eslint-disable no-unused-vars */
	Network
	/* eslint-enable no-unused-vars */
} from '../src/symbol/Network.js';
import * as sc from '../src/symbol/models.js';

/**
 * Represents a 256-bit vrf proof gamma.
 */
class ProofGamma extends ByteArray {
	static SIZE = 32;

	/**
	 * Creates a vrf proof gamma from bytes or a hex string.
	 * @param {Uint8Array|string} proofGamma Input string or byte array.
	 */
	constructor(proofGamma) {
		super(ProofGamma.SIZE, proofGamma);
	}
}

/**
 * Represents a 128-bit vrf proof verification hash.
 */
class ProofVerificationHash extends ByteArray {
	static SIZE = 16;

	/**
	 * Creates a proof verification hash from bytes or a hex string.
	 * @param {Uint8Array|string} proofVerificationHash Input string or byte array.
	 */
	constructor(proofVerificationHash) {
		super(ProofVerificationHash.SIZE, proofVerificationHash);
	}
}

/**
 * Represents a 256-bit vrf proof scalar.
 */
class ProofScalar extends ByteArray {
	static SIZE = 32;

	/**
	 * Creates a vrf proof scalar from bytes or a hex string.
	 * @param {Uint8Array|string} proofScalar Input string or byte array.
	 */
	constructor(proofScalar) {
		super(ProofScalar.SIZE, proofScalar);
	}
}

/**
 * Factory for creating Symbol blocks.
 */
export default class BlockFactory {
	/**
	 * Creates a factory for the specified network.
	 * @param {Network} network Symbol network.
	 * @param {Map<string, function>|undefined} typeRuleOverrides Type rule overrides.
	 */
	constructor(network, typeRuleOverrides = undefined) {
		this.factory = BlockFactory._buildRules(typeRuleOverrides); // eslint-disable-line no-underscore-dangle
		this.network = network;
	}

	_createAndExtend(blockDescriptor, FactoryClass) {
		const block = this.factory.createFromFactory(FactoryClass.createByName, {
			...blockDescriptor,
			network: this.network.identifier
		});

		return block;
	}

	/**
	 * Creates a block from a block descriptor.
	 * @param {object} blockDescriptor Block descriptor.
	 * @returns {object} Newly created block.
	 */
	create(blockDescriptor) {
		return this._createAndExtend(blockDescriptor, sc.BlockFactory);
	}

	static _symbolTypeConverter(value) {
		if (value instanceof Address)
			return new sc.UnresolvedAddress(value.bytes);

		return undefined;
	}

	static _buildRules(typeRuleOverrides) {
		const factory = new RuleBasedTransactionFactory(sc, this._symbolTypeConverter, typeRuleOverrides);
		factory.autodetect();

		['BlockType', 'NetworkType'].forEach(name => { factory.addEnumParser(name); });

		factory.addStructParser('VrfProof');

		const sdkTypeMapping = {
			UnresolvedAddress: Address,
			Address,
			Hash256,
			PublicKey,
			ProofGamma,
			ProofVerificationHash,
			ProofScalar
		};
		Object.keys(sdkTypeMapping).forEach(name => { factory.addPodParser(name, sdkTypeMapping[name]); });

		['BlockType', 'UnresolvedAddress'].forEach(name => {
			factory.addArrayParser(name);
		});

		return factory;
	}
}
