const { BaseValue } = require('../BaseValue');
const { ByteArray } = require('../ByteArray');
const { BufferView } = require('../utils/BufferView');
const { Writer } = require('../utils/Writer');
const arrayHelpers = require('../utils/arrayHelpers');
const converter = require('../utils/converter');

class Amount extends BaseValue {
	static SIZE = 8;

	constructor(amount = 0n) {
		super(Amount.SIZE, amount);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Amount(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class BlockDuration extends BaseValue {
	static SIZE = 8;

	constructor(blockDuration = 0n) {
		super(BlockDuration.SIZE, blockDuration);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new BlockDuration(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class BlockFeeMultiplier extends BaseValue {
	static SIZE = 4;

	constructor(blockFeeMultiplier = 0) {
		super(BlockFeeMultiplier.SIZE, blockFeeMultiplier);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new BlockFeeMultiplier(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

class Difficulty extends BaseValue {
	static SIZE = 8;

	constructor(difficulty = 0n) {
		super(Difficulty.SIZE, difficulty);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Difficulty(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class FinalizationEpoch extends BaseValue {
	static SIZE = 4;

	constructor(finalizationEpoch = 0) {
		super(FinalizationEpoch.SIZE, finalizationEpoch);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new FinalizationEpoch(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

class FinalizationPoint extends BaseValue {
	static SIZE = 4;

	constructor(finalizationPoint = 0) {
		super(FinalizationPoint.SIZE, finalizationPoint);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new FinalizationPoint(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

class Height extends BaseValue {
	static SIZE = 8;

	constructor(height = 0n) {
		super(Height.SIZE, height);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Height(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class Importance extends BaseValue {
	static SIZE = 8;

	constructor(importance = 0n) {
		super(Importance.SIZE, importance);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Importance(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class ImportanceHeight extends BaseValue {
	static SIZE = 8;

	constructor(importanceHeight = 0n) {
		super(ImportanceHeight.SIZE, importanceHeight);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new ImportanceHeight(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class UnresolvedMosaicId extends BaseValue {
	static SIZE = 8;

	constructor(unresolvedMosaicId = 0n) {
		super(UnresolvedMosaicId.SIZE, unresolvedMosaicId);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new UnresolvedMosaicId(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class MosaicId extends BaseValue {
	static SIZE = 8;

	constructor(mosaicId = 0n) {
		super(MosaicId.SIZE, mosaicId);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicId(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class Timestamp extends BaseValue {
	static SIZE = 8;

	constructor(timestamp = 0n) {
		super(Timestamp.SIZE, timestamp);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Timestamp(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class UnresolvedAddress extends ByteArray {
	static SIZE = 24;

	constructor(unresolvedAddress = new Uint8Array(24)) {
		super(UnresolvedAddress.SIZE, unresolvedAddress);
	}

	get size() {
		return 24;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new UnresolvedAddress(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 24));
	}

	serialize() {
		return this.bytes;
	}
}

class Address extends ByteArray {
	static SIZE = 24;

	constructor(address = new Uint8Array(24)) {
		super(Address.SIZE, address);
	}

	get size() {
		return 24;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Address(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 24));
	}

	serialize() {
		return this.bytes;
	}
}

class Hash256 extends ByteArray {
	static SIZE = 32;

	constructor(hash256 = new Uint8Array(32)) {
		super(Hash256.SIZE, hash256);
	}

	get size() {
		return 32;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Hash256(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 32));
	}

	serialize() {
		return this.bytes;
	}
}

class Hash512 extends ByteArray {
	static SIZE = 64;

	constructor(hash512 = new Uint8Array(64)) {
		super(Hash512.SIZE, hash512);
	}

	get size() {
		return 64;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Hash512(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 64));
	}

	serialize() {
		return this.bytes;
	}
}

class PublicKey extends ByteArray {
	static SIZE = 32;

	constructor(publicKey = new Uint8Array(32)) {
		super(PublicKey.SIZE, publicKey);
	}

	get size() {
		return 32;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new PublicKey(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 32));
	}

	serialize() {
		return this.bytes;
	}
}

class VotingPublicKey extends ByteArray {
	static SIZE = 32;

	constructor(votingPublicKey = new Uint8Array(32)) {
		super(VotingPublicKey.SIZE, votingPublicKey);
	}

	get size() {
		return 32;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new VotingPublicKey(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 32));
	}

	serialize() {
		return this.bytes;
	}
}

class Signature extends ByteArray {
	static SIZE = 64;

	constructor(signature = new Uint8Array(64)) {
		super(Signature.SIZE, signature);
	}

	get size() {
		return 64;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Signature(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 64));
	}

	serialize() {
		return this.bytes;
	}
}

class Mosaic {
	static TYPE_HINTS = {
		mosaicId: 'pod:MosaicId',
		amount: 'pod:Amount'
	};

	constructor() {
		this._mosaicId = new MosaicId();
		this._amount = new Amount();
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get amount() {
		return this._amount;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set amount(value) {
		this._amount = value;
	}

	get size() {
		let size = 0
		size += this.mosaicId.size;
		size += this.amount.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const amount = Amount.deserialize(view.buffer);
		view.shift(amount.size);

		const instance = new Mosaic();
		instance._mosaicId = mosaicId;
		instance._amount = amount;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._amount.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `amount: ${this._amount.toString()}`;
		result += ')';
		return result;
	}
}

class UnresolvedMosaic {
	static TYPE_HINTS = {
		mosaicId: 'pod:UnresolvedMosaicId',
		amount: 'pod:Amount'
	};

	constructor() {
		this._mosaicId = new UnresolvedMosaicId();
		this._amount = new Amount();
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get amount() {
		return this._amount;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set amount(value) {
		this._amount = value;
	}

	get size() {
		let size = 0
		size += this.mosaicId.size;
		size += this.amount.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const amount = Amount.deserialize(view.buffer);
		view.shift(amount.size);

		const instance = new UnresolvedMosaic();
		instance._mosaicId = mosaicId;
		instance._amount = amount;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._amount.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `amount: ${this._amount.toString()}`;
		result += ')';
		return result;
	}
}

class LinkAction {
	static UNLINK = new LinkAction(0);

	static LINK = new LinkAction(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'UNLINK', 'LINK'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return LinkAction[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class NetworkType {
	static MAINNET = new NetworkType(104);

	static TESTNET = new NetworkType(152);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			104, 152
		];
		const keys = [
			'MAINNET', 'TESTNET'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return NetworkType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class TransactionType {
	static ACCOUNT_KEY_LINK = new TransactionType(16716);

	static NODE_KEY_LINK = new TransactionType(16972);

	static AGGREGATE_COMPLETE = new TransactionType(16705);

	static AGGREGATE_BONDED = new TransactionType(16961);

	static VOTING_KEY_LINK = new TransactionType(16707);

	static VRF_KEY_LINK = new TransactionType(16963);

	static HASH_LOCK = new TransactionType(16712);

	static SECRET_LOCK = new TransactionType(16722);

	static SECRET_PROOF = new TransactionType(16978);

	static ACCOUNT_METADATA = new TransactionType(16708);

	static MOSAIC_METADATA = new TransactionType(16964);

	static NAMESPACE_METADATA = new TransactionType(17220);

	static MOSAIC_DEFINITION = new TransactionType(16717);

	static MOSAIC_SUPPLY_CHANGE = new TransactionType(16973);

	static MOSAIC_SUPPLY_REVOCATION = new TransactionType(17229);

	static MULTISIG_ACCOUNT_MODIFICATION = new TransactionType(16725);

	static ADDRESS_ALIAS = new TransactionType(16974);

	static MOSAIC_ALIAS = new TransactionType(17230);

	static NAMESPACE_REGISTRATION = new TransactionType(16718);

	static ACCOUNT_ADDRESS_RESTRICTION = new TransactionType(16720);

	static ACCOUNT_MOSAIC_RESTRICTION = new TransactionType(16976);

	static ACCOUNT_OPERATION_RESTRICTION = new TransactionType(17232);

	static MOSAIC_ADDRESS_RESTRICTION = new TransactionType(16977);

	static MOSAIC_GLOBAL_RESTRICTION = new TransactionType(16721);

	static TRANSFER = new TransactionType(16724);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			16716, 16972, 16705, 16961, 16707, 16963, 16712, 16722, 16978, 16708, 16964, 17220, 16717, 16973, 17229, 16725, 16974, 17230, 
			16718, 16720, 16976, 17232, 16977, 16721, 16724
		];
		const keys = [
			'ACCOUNT_KEY_LINK', 'NODE_KEY_LINK', 'AGGREGATE_COMPLETE', 'AGGREGATE_BONDED', 'VOTING_KEY_LINK', 'VRF_KEY_LINK', 'HASH_LOCK', 
			'SECRET_LOCK', 'SECRET_PROOF', 'ACCOUNT_METADATA', 'MOSAIC_METADATA', 'NAMESPACE_METADATA', 'MOSAIC_DEFINITION', 
			'MOSAIC_SUPPLY_CHANGE', 'MOSAIC_SUPPLY_REVOCATION', 'MULTISIG_ACCOUNT_MODIFICATION', 'ADDRESS_ALIAS', 'MOSAIC_ALIAS', 
			'NAMESPACE_REGISTRATION', 'ACCOUNT_ADDRESS_RESTRICTION', 'ACCOUNT_MOSAIC_RESTRICTION', 'ACCOUNT_OPERATION_RESTRICTION', 
			'MOSAIC_ADDRESS_RESTRICTION', 'MOSAIC_GLOBAL_RESTRICTION', 'TRANSFER'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return TransactionType[keys[index]];
	}

	get size() {
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}
}

class Transaction {
	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = 0;
		this._network = NetworkType.MAINNET;
		this._type = TransactionType.ACCOUNT_KEY_LINK;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);

		const instance = new Transaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedTransaction {
	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = 0;
		this._network = NetworkType.MAINNET;
		this._type = TransactionType.ACCOUNT_KEY_LINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);

		const instance = new EmbeddedTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += ')';
		return result;
	}
}

class ProofGamma extends ByteArray {
	static SIZE = 32;

	constructor(proofGamma = new Uint8Array(32)) {
		super(ProofGamma.SIZE, proofGamma);
	}

	get size() {
		return 32;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new ProofGamma(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 32));
	}

	serialize() {
		return this.bytes;
	}
}

class ProofVerificationHash extends ByteArray {
	static SIZE = 16;

	constructor(proofVerificationHash = new Uint8Array(16)) {
		super(ProofVerificationHash.SIZE, proofVerificationHash);
	}

	get size() {
		return 16;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new ProofVerificationHash(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 16));
	}

	serialize() {
		return this.bytes;
	}
}

class ProofScalar extends ByteArray {
	static SIZE = 32;

	constructor(proofScalar = new Uint8Array(32)) {
		super(ProofScalar.SIZE, proofScalar);
	}

	get size() {
		return 32;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new ProofScalar(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 32));
	}

	serialize() {
		return this.bytes;
	}
}

class BlockType {
	static NEMESIS = new BlockType(32835);

	static NORMAL = new BlockType(33091);

	static IMPORTANCE = new BlockType(33347);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			32835, 33091, 33347
		];
		const keys = [
			'NEMESIS', 'NORMAL', 'IMPORTANCE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return BlockType[keys[index]];
	}

	get size() {
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}
}

class VrfProof {
	static TYPE_HINTS = {
		gamma: 'pod:ProofGamma',
		verificationHash: 'pod:ProofVerificationHash',
		scalar: 'pod:ProofScalar'
	};

	constructor() {
		this._gamma = new ProofGamma();
		this._verificationHash = new ProofVerificationHash();
		this._scalar = new ProofScalar();
	}

	get gamma() {
		return this._gamma;
	}

	get verificationHash() {
		return this._verificationHash;
	}

	get scalar() {
		return this._scalar;
	}

	set gamma(value) {
		this._gamma = value;
	}

	set verificationHash(value) {
		this._verificationHash = value;
	}

	set scalar(value) {
		this._scalar = value;
	}

	get size() {
		let size = 0
		size += this.gamma.size;
		size += this.verificationHash.size;
		size += this.scalar.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const gamma = ProofGamma.deserialize(view.buffer);
		view.shift(gamma.size);
		const verificationHash = ProofVerificationHash.deserialize(view.buffer);
		view.shift(verificationHash.size);
		const scalar = ProofScalar.deserialize(view.buffer);
		view.shift(scalar.size);

		const instance = new VrfProof();
		instance._gamma = gamma;
		instance._verificationHash = verificationHash;
		instance._scalar = scalar;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._gamma.serialize());
		buffer_.write(this._verificationHash.serialize());
		buffer_.write(this._scalar.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `gamma: ${this._gamma.toString()}`;
		result += `verificationHash: ${this._verificationHash.toString()}`;
		result += `scalar: ${this._scalar.toString()}`;
		result += ')';
		return result;
	}
}

class Block {
	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:BlockType',
		height: 'pod:Height',
		timestamp: 'pod:Timestamp',
		difficulty: 'pod:Difficulty',
		generationHashProof: 'struct:VrfProof',
		previousBlockHash: 'pod:Hash256',
		transactionsHash: 'pod:Hash256',
		receiptsHash: 'pod:Hash256',
		stateHash: 'pod:Hash256',
		beneficiaryAddress: 'pod:Address',
		feeMultiplier: 'pod:BlockFeeMultiplier'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = 0;
		this._network = NetworkType.MAINNET;
		this._type = BlockType.NEMESIS;
		this._height = new Height();
		this._timestamp = new Timestamp();
		this._difficulty = new Difficulty();
		this._generationHashProof = new VrfProof();
		this._previousBlockHash = new Hash256();
		this._transactionsHash = new Hash256();
		this._receiptsHash = new Hash256();
		this._stateHash = new Hash256();
		this._beneficiaryAddress = new Address();
		this._feeMultiplier = new BlockFeeMultiplier();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get height() {
		return this._height;
	}

	get timestamp() {
		return this._timestamp;
	}

	get difficulty() {
		return this._difficulty;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	get stateHash() {
		return this._stateHash;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set height(value) {
		this._height = value;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.height.size;
		size += this.timestamp.size;
		size += this.difficulty.size;
		size += this.generationHashProof.size;
		size += this.previousBlockHash.size;
		size += this.transactionsHash.size;
		size += this.receiptsHash.size;
		size += this.stateHash.size;
		size += this.beneficiaryAddress.size;
		size += this.feeMultiplier.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = BlockType.deserialize(view.buffer);
		view.shift(type.size);
		const height = Height.deserialize(view.buffer);
		view.shift(height.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shift(timestamp.size);
		const difficulty = Difficulty.deserialize(view.buffer);
		view.shift(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shift(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shift(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shift(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shift(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shift(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shift(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserialize(view.buffer);
		view.shift(feeMultiplier.size);

		const instance = new Block();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._height = height;
		instance._timestamp = timestamp;
		instance._difficulty = difficulty;
		instance._generationHashProof = generationHashProof;
		instance._previousBlockHash = previousBlockHash;
		instance._transactionsHash = transactionsHash;
		instance._receiptsHash = receiptsHash;
		instance._stateHash = stateHash;
		instance._beneficiaryAddress = beneficiaryAddress;
		instance._feeMultiplier = feeMultiplier;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._height.serialize());
		buffer_.write(this._timestamp.serialize());
		buffer_.write(this._difficulty.serialize());
		buffer_.write(this._generationHashProof.serialize());
		buffer_.write(this._previousBlockHash.serialize());
		buffer_.write(this._transactionsHash.serialize());
		buffer_.write(this._receiptsHash.serialize());
		buffer_.write(this._stateHash.serialize());
		buffer_.write(this._beneficiaryAddress.serialize());
		buffer_.write(this._feeMultiplier.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `height: ${this._height.toString()}`;
		result += `timestamp: ${this._timestamp.toString()}`;
		result += `difficulty: ${this._difficulty.toString()}`;
		result += `generationHashProof: ${this._generationHashProof.toString()}`;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}`;
		result += `transactionsHash: ${this._transactionsHash.toString()}`;
		result += `receiptsHash: ${this._receiptsHash.toString()}`;
		result += `stateHash: ${this._stateHash.toString()}`;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}`;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}`;
		result += ')';
		return result;
	}
}

class NemesisBlock {
	static BLOCK_VERSION = 1;

	static BLOCK_TYPE = BlockType.NEMESIS;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:BlockType',
		height: 'pod:Height',
		timestamp: 'pod:Timestamp',
		difficulty: 'pod:Difficulty',
		generationHashProof: 'struct:VrfProof',
		previousBlockHash: 'pod:Hash256',
		transactionsHash: 'pod:Hash256',
		receiptsHash: 'pod:Hash256',
		stateHash: 'pod:Hash256',
		beneficiaryAddress: 'pod:Address',
		feeMultiplier: 'pod:BlockFeeMultiplier',
		totalVotingBalance: 'pod:Amount',
		previousImportanceBlockHash: 'pod:Hash256',
		transactions: 'array[Transaction]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = NemesisBlock.BLOCK_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NemesisBlock.BLOCK_TYPE;
		this._height = new Height();
		this._timestamp = new Timestamp();
		this._difficulty = new Difficulty();
		this._generationHashProof = new VrfProof();
		this._previousBlockHash = new Hash256();
		this._transactionsHash = new Hash256();
		this._receiptsHash = new Hash256();
		this._stateHash = new Hash256();
		this._beneficiaryAddress = new Address();
		this._feeMultiplier = new BlockFeeMultiplier();
		this._votingEligibleAccountsCount = 0;
		this._harvestingEligibleAccountsCount = 0n;
		this._totalVotingBalance = new Amount();
		this._previousImportanceBlockHash = new Hash256();
		this._transactions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get height() {
		return this._height;
	}

	get timestamp() {
		return this._timestamp;
	}

	get difficulty() {
		return this._difficulty;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	get stateHash() {
		return this._stateHash;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	get votingEligibleAccountsCount() {
		return this._votingEligibleAccountsCount;
	}

	get harvestingEligibleAccountsCount() {
		return this._harvestingEligibleAccountsCount;
	}

	get totalVotingBalance() {
		return this._totalVotingBalance;
	}

	get previousImportanceBlockHash() {
		return this._previousImportanceBlockHash;
	}

	get transactions() {
		return this._transactions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set height(value) {
		this._height = value;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	set votingEligibleAccountsCount(value) {
		this._votingEligibleAccountsCount = value;
	}

	set harvestingEligibleAccountsCount(value) {
		this._harvestingEligibleAccountsCount = value;
	}

	set totalVotingBalance(value) {
		this._totalVotingBalance = value;
	}

	set previousImportanceBlockHash(value) {
		this._previousImportanceBlockHash = value;
	}

	set transactions(value) {
		this._transactions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.height.size;
		size += this.timestamp.size;
		size += this.difficulty.size;
		size += this.generationHashProof.size;
		size += this.previousBlockHash.size;
		size += this.transactionsHash.size;
		size += this.receiptsHash.size;
		size += this.stateHash.size;
		size += this.beneficiaryAddress.size;
		size += this.feeMultiplier.size;
		size += 4;
		size += 8;
		size += this.totalVotingBalance.size;
		size += this.previousImportanceBlockHash.size;
		size += this.transactions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = BlockType.deserialize(view.buffer);
		view.shift(type.size);
		const height = Height.deserialize(view.buffer);
		view.shift(height.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shift(timestamp.size);
		const difficulty = Difficulty.deserialize(view.buffer);
		view.shift(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shift(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shift(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shift(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shift(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shift(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shift(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserialize(view.buffer);
		view.shift(feeMultiplier.size);
		const votingEligibleAccountsCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const harvestingEligibleAccountsCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const totalVotingBalance = Amount.deserialize(view.buffer);
		view.shift(totalVotingBalance.size);
		const previousImportanceBlockHash = Hash256.deserialize(view.buffer);
		view.shift(previousImportanceBlockHash.size);
		const transactions = arrayHelpers.read_array(view.buffer, Transaction);
		view.shift(transactions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new NemesisBlock();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._height = height;
		instance._timestamp = timestamp;
		instance._difficulty = difficulty;
		instance._generationHashProof = generationHashProof;
		instance._previousBlockHash = previousBlockHash;
		instance._transactionsHash = transactionsHash;
		instance._receiptsHash = receiptsHash;
		instance._stateHash = stateHash;
		instance._beneficiaryAddress = beneficiaryAddress;
		instance._feeMultiplier = feeMultiplier;
		instance._votingEligibleAccountsCount = votingEligibleAccountsCount;
		instance._harvestingEligibleAccountsCount = harvestingEligibleAccountsCount;
		instance._totalVotingBalance = totalVotingBalance;
		instance._previousImportanceBlockHash = previousImportanceBlockHash;
		instance._transactions = transactions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._height.serialize());
		buffer_.write(this._timestamp.serialize());
		buffer_.write(this._difficulty.serialize());
		buffer_.write(this._generationHashProof.serialize());
		buffer_.write(this._previousBlockHash.serialize());
		buffer_.write(this._transactionsHash.serialize());
		buffer_.write(this._receiptsHash.serialize());
		buffer_.write(this._stateHash.serialize());
		buffer_.write(this._beneficiaryAddress.serialize());
		buffer_.write(this._feeMultiplier.serialize());
		buffer_.write(converter.intToBytes(this._votingEligibleAccountsCount, 4, false));
		buffer_.write(converter.intToBytes(this._harvestingEligibleAccountsCount, 8, false));
		buffer_.write(this._totalVotingBalance.serialize());
		buffer_.write(this._previousImportanceBlockHash.serialize());
		arrayHelpers.write_array(buffer_, this._transactions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `height: ${this._height.toString()}`;
		result += `timestamp: ${this._timestamp.toString()}`;
		result += `difficulty: ${this._difficulty.toString()}`;
		result += `generationHashProof: ${this._generationHashProof.toString()}`;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}`;
		result += `transactionsHash: ${this._transactionsHash.toString()}`;
		result += `receiptsHash: ${this._receiptsHash.toString()}`;
		result += `stateHash: ${this._stateHash.toString()}`;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}`;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}`;
		result += `votingEligibleAccountsCount: ${"0x".concat(this._votingEligibleAccountsCount.toString(16))}`;
		result += `harvestingEligibleAccountsCount: ${"0x".concat(this._harvestingEligibleAccountsCount.toString(16))}`;
		result += `totalVotingBalance: ${this._totalVotingBalance.toString()}`;
		result += `previousImportanceBlockHash: ${this._previousImportanceBlockHash.toString()}`;
		result += `transactions: ${this._transactions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class NormalBlock {
	static BLOCK_VERSION = 1;

	static BLOCK_TYPE = BlockType.NORMAL;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:BlockType',
		height: 'pod:Height',
		timestamp: 'pod:Timestamp',
		difficulty: 'pod:Difficulty',
		generationHashProof: 'struct:VrfProof',
		previousBlockHash: 'pod:Hash256',
		transactionsHash: 'pod:Hash256',
		receiptsHash: 'pod:Hash256',
		stateHash: 'pod:Hash256',
		beneficiaryAddress: 'pod:Address',
		feeMultiplier: 'pod:BlockFeeMultiplier',
		transactions: 'array[Transaction]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = NormalBlock.BLOCK_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NormalBlock.BLOCK_TYPE;
		this._height = new Height();
		this._timestamp = new Timestamp();
		this._difficulty = new Difficulty();
		this._generationHashProof = new VrfProof();
		this._previousBlockHash = new Hash256();
		this._transactionsHash = new Hash256();
		this._receiptsHash = new Hash256();
		this._stateHash = new Hash256();
		this._beneficiaryAddress = new Address();
		this._feeMultiplier = new BlockFeeMultiplier();
		this._transactions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._blockHeaderReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get height() {
		return this._height;
	}

	get timestamp() {
		return this._timestamp;
	}

	get difficulty() {
		return this._difficulty;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	get stateHash() {
		return this._stateHash;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	get transactions() {
		return this._transactions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set height(value) {
		this._height = value;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	set transactions(value) {
		this._transactions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.height.size;
		size += this.timestamp.size;
		size += this.difficulty.size;
		size += this.generationHashProof.size;
		size += this.previousBlockHash.size;
		size += this.transactionsHash.size;
		size += this.receiptsHash.size;
		size += this.stateHash.size;
		size += this.beneficiaryAddress.size;
		size += this.feeMultiplier.size;
		size += 4;
		size += this.transactions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = BlockType.deserialize(view.buffer);
		view.shift(type.size);
		const height = Height.deserialize(view.buffer);
		view.shift(height.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shift(timestamp.size);
		const difficulty = Difficulty.deserialize(view.buffer);
		view.shift(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shift(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shift(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shift(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shift(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shift(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shift(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserialize(view.buffer);
		view.shift(feeMultiplier.size);
		const blockHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (blockHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${blockHeaderReserved_1})`);
		const transactions = arrayHelpers.read_array(view.buffer, Transaction);
		view.shift(transactions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new NormalBlock();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._height = height;
		instance._timestamp = timestamp;
		instance._difficulty = difficulty;
		instance._generationHashProof = generationHashProof;
		instance._previousBlockHash = previousBlockHash;
		instance._transactionsHash = transactionsHash;
		instance._receiptsHash = receiptsHash;
		instance._stateHash = stateHash;
		instance._beneficiaryAddress = beneficiaryAddress;
		instance._feeMultiplier = feeMultiplier;
		instance._transactions = transactions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._height.serialize());
		buffer_.write(this._timestamp.serialize());
		buffer_.write(this._difficulty.serialize());
		buffer_.write(this._generationHashProof.serialize());
		buffer_.write(this._previousBlockHash.serialize());
		buffer_.write(this._transactionsHash.serialize());
		buffer_.write(this._receiptsHash.serialize());
		buffer_.write(this._stateHash.serialize());
		buffer_.write(this._beneficiaryAddress.serialize());
		buffer_.write(this._feeMultiplier.serialize());
		buffer_.write(converter.intToBytes(this._blockHeaderReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._transactions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `height: ${this._height.toString()}`;
		result += `timestamp: ${this._timestamp.toString()}`;
		result += `difficulty: ${this._difficulty.toString()}`;
		result += `generationHashProof: ${this._generationHashProof.toString()}`;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}`;
		result += `transactionsHash: ${this._transactionsHash.toString()}`;
		result += `receiptsHash: ${this._receiptsHash.toString()}`;
		result += `stateHash: ${this._stateHash.toString()}`;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}`;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}`;
		result += `transactions: ${this._transactions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class ImportanceBlock {
	static BLOCK_VERSION = 1;

	static BLOCK_TYPE = BlockType.IMPORTANCE;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:BlockType',
		height: 'pod:Height',
		timestamp: 'pod:Timestamp',
		difficulty: 'pod:Difficulty',
		generationHashProof: 'struct:VrfProof',
		previousBlockHash: 'pod:Hash256',
		transactionsHash: 'pod:Hash256',
		receiptsHash: 'pod:Hash256',
		stateHash: 'pod:Hash256',
		beneficiaryAddress: 'pod:Address',
		feeMultiplier: 'pod:BlockFeeMultiplier',
		totalVotingBalance: 'pod:Amount',
		previousImportanceBlockHash: 'pod:Hash256',
		transactions: 'array[Transaction]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = ImportanceBlock.BLOCK_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = ImportanceBlock.BLOCK_TYPE;
		this._height = new Height();
		this._timestamp = new Timestamp();
		this._difficulty = new Difficulty();
		this._generationHashProof = new VrfProof();
		this._previousBlockHash = new Hash256();
		this._transactionsHash = new Hash256();
		this._receiptsHash = new Hash256();
		this._stateHash = new Hash256();
		this._beneficiaryAddress = new Address();
		this._feeMultiplier = new BlockFeeMultiplier();
		this._votingEligibleAccountsCount = 0;
		this._harvestingEligibleAccountsCount = 0n;
		this._totalVotingBalance = new Amount();
		this._previousImportanceBlockHash = new Hash256();
		this._transactions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get height() {
		return this._height;
	}

	get timestamp() {
		return this._timestamp;
	}

	get difficulty() {
		return this._difficulty;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	get stateHash() {
		return this._stateHash;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	get votingEligibleAccountsCount() {
		return this._votingEligibleAccountsCount;
	}

	get harvestingEligibleAccountsCount() {
		return this._harvestingEligibleAccountsCount;
	}

	get totalVotingBalance() {
		return this._totalVotingBalance;
	}

	get previousImportanceBlockHash() {
		return this._previousImportanceBlockHash;
	}

	get transactions() {
		return this._transactions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set height(value) {
		this._height = value;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	set votingEligibleAccountsCount(value) {
		this._votingEligibleAccountsCount = value;
	}

	set harvestingEligibleAccountsCount(value) {
		this._harvestingEligibleAccountsCount = value;
	}

	set totalVotingBalance(value) {
		this._totalVotingBalance = value;
	}

	set previousImportanceBlockHash(value) {
		this._previousImportanceBlockHash = value;
	}

	set transactions(value) {
		this._transactions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.height.size;
		size += this.timestamp.size;
		size += this.difficulty.size;
		size += this.generationHashProof.size;
		size += this.previousBlockHash.size;
		size += this.transactionsHash.size;
		size += this.receiptsHash.size;
		size += this.stateHash.size;
		size += this.beneficiaryAddress.size;
		size += this.feeMultiplier.size;
		size += 4;
		size += 8;
		size += this.totalVotingBalance.size;
		size += this.previousImportanceBlockHash.size;
		size += this.transactions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = BlockType.deserialize(view.buffer);
		view.shift(type.size);
		const height = Height.deserialize(view.buffer);
		view.shift(height.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shift(timestamp.size);
		const difficulty = Difficulty.deserialize(view.buffer);
		view.shift(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shift(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shift(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shift(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shift(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shift(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shift(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserialize(view.buffer);
		view.shift(feeMultiplier.size);
		const votingEligibleAccountsCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const harvestingEligibleAccountsCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const totalVotingBalance = Amount.deserialize(view.buffer);
		view.shift(totalVotingBalance.size);
		const previousImportanceBlockHash = Hash256.deserialize(view.buffer);
		view.shift(previousImportanceBlockHash.size);
		const transactions = arrayHelpers.read_array(view.buffer, Transaction);
		view.shift(transactions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new ImportanceBlock();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._height = height;
		instance._timestamp = timestamp;
		instance._difficulty = difficulty;
		instance._generationHashProof = generationHashProof;
		instance._previousBlockHash = previousBlockHash;
		instance._transactionsHash = transactionsHash;
		instance._receiptsHash = receiptsHash;
		instance._stateHash = stateHash;
		instance._beneficiaryAddress = beneficiaryAddress;
		instance._feeMultiplier = feeMultiplier;
		instance._votingEligibleAccountsCount = votingEligibleAccountsCount;
		instance._harvestingEligibleAccountsCount = harvestingEligibleAccountsCount;
		instance._totalVotingBalance = totalVotingBalance;
		instance._previousImportanceBlockHash = previousImportanceBlockHash;
		instance._transactions = transactions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._height.serialize());
		buffer_.write(this._timestamp.serialize());
		buffer_.write(this._difficulty.serialize());
		buffer_.write(this._generationHashProof.serialize());
		buffer_.write(this._previousBlockHash.serialize());
		buffer_.write(this._transactionsHash.serialize());
		buffer_.write(this._receiptsHash.serialize());
		buffer_.write(this._stateHash.serialize());
		buffer_.write(this._beneficiaryAddress.serialize());
		buffer_.write(this._feeMultiplier.serialize());
		buffer_.write(converter.intToBytes(this._votingEligibleAccountsCount, 4, false));
		buffer_.write(converter.intToBytes(this._harvestingEligibleAccountsCount, 8, false));
		buffer_.write(this._totalVotingBalance.serialize());
		buffer_.write(this._previousImportanceBlockHash.serialize());
		arrayHelpers.write_array(buffer_, this._transactions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `height: ${this._height.toString()}`;
		result += `timestamp: ${this._timestamp.toString()}`;
		result += `difficulty: ${this._difficulty.toString()}`;
		result += `generationHashProof: ${this._generationHashProof.toString()}`;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}`;
		result += `transactionsHash: ${this._transactionsHash.toString()}`;
		result += `receiptsHash: ${this._receiptsHash.toString()}`;
		result += `stateHash: ${this._stateHash.toString()}`;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}`;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}`;
		result += `votingEligibleAccountsCount: ${"0x".concat(this._votingEligibleAccountsCount.toString(16))}`;
		result += `harvestingEligibleAccountsCount: ${"0x".concat(this._harvestingEligibleAccountsCount.toString(16))}`;
		result += `totalVotingBalance: ${this._totalVotingBalance.toString()}`;
		result += `previousImportanceBlockHash: ${this._previousImportanceBlockHash.toString()}`;
		result += `transactions: ${this._transactions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class FinalizationRound {
	static TYPE_HINTS = {
		epoch: 'pod:FinalizationEpoch',
		point: 'pod:FinalizationPoint'
	};

	constructor() {
		this._epoch = new FinalizationEpoch();
		this._point = new FinalizationPoint();
	}

	get epoch() {
		return this._epoch;
	}

	get point() {
		return this._point;
	}

	set epoch(value) {
		this._epoch = value;
	}

	set point(value) {
		this._point = value;
	}

	get size() {
		let size = 0
		size += this.epoch.size;
		size += this.point.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const epoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(epoch.size);
		const point = FinalizationPoint.deserialize(view.buffer);
		view.shift(point.size);

		const instance = new FinalizationRound();
		instance._epoch = epoch;
		instance._point = point;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._epoch.serialize());
		buffer_.write(this._point.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `epoch: ${this._epoch.toString()}`;
		result += `point: ${this._point.toString()}`;
		result += ')';
		return result;
	}
}

class FinalizedBlockHeader {
	static TYPE_HINTS = {
		round: 'struct:FinalizationRound',
		height: 'pod:Height',
		hash: 'pod:Hash256'
	};

	constructor() {
		this._round = new FinalizationRound();
		this._height = new Height();
		this._hash = new Hash256();
	}

	get round() {
		return this._round;
	}

	get height() {
		return this._height;
	}

	get hash() {
		return this._hash;
	}

	set round(value) {
		this._round = value;
	}

	set height(value) {
		this._height = value;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() {
		let size = 0
		size += this.round.size;
		size += this.height.size;
		size += this.hash.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const round = FinalizationRound.deserialize(view.buffer);
		view.shift(round.size);
		const height = Height.deserialize(view.buffer);
		view.shift(height.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shift(hash.size);

		const instance = new FinalizedBlockHeader();
		instance._round = round;
		instance._height = height;
		instance._hash = hash;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._round.serialize());
		buffer_.write(this._height.serialize());
		buffer_.write(this._hash.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `round: ${this._round.toString()}`;
		result += `height: ${this._height.toString()}`;
		result += `hash: ${this._hash.toString()}`;
		result += ')';
		return result;
	}
}

class ReceiptType {
	static MOSAIC_RENTAL_FEE = new ReceiptType(4685);

	static NAMESPACE_RENTAL_FEE = new ReceiptType(4942);

	static HARVEST_FEE = new ReceiptType(8515);

	static LOCK_HASH_COMPLETED = new ReceiptType(8776);

	static LOCK_HASH_EXPIRED = new ReceiptType(9032);

	static LOCK_SECRET_COMPLETED = new ReceiptType(8786);

	static LOCK_SECRET_EXPIRED = new ReceiptType(9042);

	static LOCK_HASH_CREATED = new ReceiptType(12616);

	static LOCK_SECRET_CREATED = new ReceiptType(12626);

	static MOSAIC_EXPIRED = new ReceiptType(16717);

	static NAMESPACE_EXPIRED = new ReceiptType(16718);

	static NAMESPACE_DELETED = new ReceiptType(16974);

	static INFLATION = new ReceiptType(20803);

	static TRANSACTION_GROUP = new ReceiptType(57667);

	static ADDRESS_ALIAS_RESOLUTION = new ReceiptType(61763);

	static MOSAIC_ALIAS_RESOLUTION = new ReceiptType(62019);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			4685, 4942, 8515, 8776, 9032, 8786, 9042, 12616, 12626, 16717, 16718, 16974, 20803, 57667, 61763, 62019
		];
		const keys = [
			'MOSAIC_RENTAL_FEE', 'NAMESPACE_RENTAL_FEE', 'HARVEST_FEE', 'LOCK_HASH_COMPLETED', 'LOCK_HASH_EXPIRED', 'LOCK_SECRET_COMPLETED', 
			'LOCK_SECRET_EXPIRED', 'LOCK_HASH_CREATED', 'LOCK_SECRET_CREATED', 'MOSAIC_EXPIRED', 'NAMESPACE_EXPIRED', 'NAMESPACE_DELETED', 
			'INFLATION', 'TRANSACTION_GROUP', 'ADDRESS_ALIAS_RESOLUTION', 'MOSAIC_ALIAS_RESOLUTION'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return ReceiptType[keys[index]];
	}

	get size() {
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}
}

class Receipt {
	static TYPE_HINTS = {
		type: 'enum:ReceiptType'
	};

	constructor() {
		this._version = 0;
		this._type = ReceiptType.MOSAIC_RENTAL_FEE;
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);

		const instance = new Receipt();
		instance._version = version;
		instance._type = type;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += ')';
		return result;
	}
}

class HarvestFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.HARVEST_FEE;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = HarvestFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new HarvestFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class InflationReceipt {
	static RECEIPT_TYPE = ReceiptType.INFLATION;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic'
	};

	constructor() {
		this._version = 0;
		this._type = InflationReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);

		const instance = new InflationReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += ')';
		return result;
	}
}

class LockHashCreatedFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.LOCK_HASH_CREATED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = LockHashCreatedFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new LockHashCreatedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class LockHashCompletedFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.LOCK_HASH_COMPLETED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = LockHashCompletedFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new LockHashCompletedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class LockHashExpiredFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.LOCK_HASH_EXPIRED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = LockHashExpiredFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new LockHashExpiredFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class LockSecretCreatedFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.LOCK_SECRET_CREATED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = LockSecretCreatedFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new LockSecretCreatedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class LockSecretCompletedFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.LOCK_SECRET_COMPLETED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = LockSecretCompletedFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new LockSecretCompletedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class LockSecretExpiredFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.LOCK_SECRET_EXPIRED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		targetAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = LockSecretExpiredFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._targetAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new LockSecretExpiredFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicExpiredReceipt {
	static RECEIPT_TYPE = ReceiptType.MOSAIC_EXPIRED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		artifactId: 'pod:MosaicId'
	};

	constructor() {
		this._version = 0;
		this._type = MosaicExpiredReceipt.RECEIPT_TYPE;
		this._artifactId = new MosaicId();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get artifactId() {
		return this._artifactId;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set artifactId(value) {
		this._artifactId = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.artifactId.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const artifactId = MosaicId.deserialize(view.buffer);
		view.shift(artifactId.size);

		const instance = new MosaicExpiredReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._artifactId.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `artifactId: ${this._artifactId.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicRentalFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.MOSAIC_RENTAL_FEE;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		senderAddress: 'pod:Address',
		recipientAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = MosaicRentalFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._senderAddress = new Address();
		this._recipientAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get senderAddress() {
		return this._senderAddress;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set senderAddress(value) {
		this._senderAddress = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.senderAddress.size;
		size += this.recipientAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const senderAddress = Address.deserialize(view.buffer);
		view.shift(senderAddress.size);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shift(recipientAddress.size);

		const instance = new MosaicRentalFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._senderAddress = senderAddress;
		instance._recipientAddress = recipientAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._senderAddress.serialize());
		buffer_.write(this._recipientAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `senderAddress: ${this._senderAddress.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += ')';
		return result;
	}
}

class NamespaceId extends BaseValue {
	static SIZE = 8;

	constructor(namespaceId = 0n) {
		super(NamespaceId.SIZE, namespaceId);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new NamespaceId(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class NamespaceRegistrationType {
	static ROOT = new NamespaceRegistrationType(0);

	static CHILD = new NamespaceRegistrationType(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'ROOT', 'CHILD'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return NamespaceRegistrationType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class AliasAction {
	static UNLINK = new AliasAction(0);

	static LINK = new AliasAction(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'UNLINK', 'LINK'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return AliasAction[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class NamespaceExpiredReceipt {
	static RECEIPT_TYPE = ReceiptType.NAMESPACE_EXPIRED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		artifactId: 'pod:NamespaceId'
	};

	constructor() {
		this._version = 0;
		this._type = NamespaceExpiredReceipt.RECEIPT_TYPE;
		this._artifactId = new NamespaceId();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get artifactId() {
		return this._artifactId;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set artifactId(value) {
		this._artifactId = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.artifactId.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const artifactId = NamespaceId.deserialize(view.buffer);
		view.shift(artifactId.size);

		const instance = new NamespaceExpiredReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._artifactId.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `artifactId: ${this._artifactId.toString()}`;
		result += ')';
		return result;
	}
}

class NamespaceDeletedReceipt {
	static RECEIPT_TYPE = ReceiptType.NAMESPACE_DELETED;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		artifactId: 'pod:NamespaceId'
	};

	constructor() {
		this._version = 0;
		this._type = NamespaceDeletedReceipt.RECEIPT_TYPE;
		this._artifactId = new NamespaceId();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get artifactId() {
		return this._artifactId;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set artifactId(value) {
		this._artifactId = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.artifactId.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const artifactId = NamespaceId.deserialize(view.buffer);
		view.shift(artifactId.size);

		const instance = new NamespaceDeletedReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._artifactId.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `artifactId: ${this._artifactId.toString()}`;
		result += ')';
		return result;
	}
}

class NamespaceRentalFeeReceipt {
	static RECEIPT_TYPE = ReceiptType.NAMESPACE_RENTAL_FEE;

	static TYPE_HINTS = {
		type: 'enum:ReceiptType',
		mosaic: 'struct:Mosaic',
		senderAddress: 'pod:Address',
		recipientAddress: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._type = NamespaceRentalFeeReceipt.RECEIPT_TYPE;
		this._mosaic = new Mosaic();
		this._senderAddress = new Address();
		this._recipientAddress = new Address();
	}

	get version() {
		return this._version;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get senderAddress() {
		return this._senderAddress;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set version(value) {
		this._version = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set senderAddress(value) {
		this._senderAddress = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.senderAddress.size;
		size += this.recipientAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const senderAddress = Address.deserialize(view.buffer);
		view.shift(senderAddress.size);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shift(recipientAddress.size);

		const instance = new NamespaceRentalFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._senderAddress = senderAddress;
		instance._recipientAddress = recipientAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._senderAddress.serialize());
		buffer_.write(this._recipientAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `senderAddress: ${this._senderAddress.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += ')';
		return result;
	}
}

class ReceiptSource {
	static TYPE_HINTS = {
	};

	constructor() {
		this._primaryId = 0;
		this._secondaryId = 0;
	}

	get primaryId() {
		return this._primaryId;
	}

	get secondaryId() {
		return this._secondaryId;
	}

	set primaryId(value) {
		this._primaryId = value;
	}

	set secondaryId(value) {
		this._secondaryId = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const primaryId = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const secondaryId = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);

		const instance = new ReceiptSource();
		instance._primaryId = primaryId;
		instance._secondaryId = secondaryId;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._primaryId, 4, false));
		buffer_.write(converter.intToBytes(this._secondaryId, 4, false));
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `primaryId: ${"0x".concat(this._primaryId.toString(16))}`;
		result += `secondaryId: ${"0x".concat(this._secondaryId.toString(16))}`;
		result += ')';
		return result;
	}
}

class AddressResolutionEntry {
	static TYPE_HINTS = {
		source: 'struct:ReceiptSource',
		resolvedValue: 'pod:Address'
	};

	constructor() {
		this._source = new ReceiptSource();
		this._resolvedValue = new Address();
	}

	get source() {
		return this._source;
	}

	get resolvedValue() {
		return this._resolvedValue;
	}

	set source(value) {
		this._source = value;
	}

	set resolvedValue(value) {
		this._resolvedValue = value;
	}

	get size() {
		let size = 0
		size += this.source.size;
		size += this.resolvedValue.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const source = ReceiptSource.deserialize(view.buffer);
		view.shift(source.size);
		const resolvedValue = Address.deserialize(view.buffer);
		view.shift(resolvedValue.size);

		const instance = new AddressResolutionEntry();
		instance._source = source;
		instance._resolvedValue = resolvedValue;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._source.serialize());
		buffer_.write(this._resolvedValue.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `source: ${this._source.toString()}`;
		result += `resolvedValue: ${this._resolvedValue.toString()}`;
		result += ')';
		return result;
	}
}

class AddressResolutionStatement {
	static TYPE_HINTS = {
		unresolved: 'pod:UnresolvedAddress',
		resolutionEntries: 'array[AddressResolutionEntry]'
	};

	constructor() {
		this._unresolved = new UnresolvedAddress();
		this._resolutionEntries = [];
	}

	get unresolved() {
		return this._unresolved;
	}

	get resolutionEntries() {
		return this._resolutionEntries;
	}

	set unresolved(value) {
		this._unresolved = value;
	}

	set resolutionEntries(value) {
		this._resolutionEntries = value;
	}

	get size() {
		let size = 0
		size += this.unresolved.size;
		size += 4;
		size += this.resolutionEntries.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const unresolved = UnresolvedAddress.deserialize(view.buffer);
		view.shift(unresolved.size);
		const resolutionEntriesCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const resolutionEntries = arrayHelpers.read_array_count(view.buffer, AddressResolutionEntry, resolutionEntriesCount);
		view.shift(resolutionEntries.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AddressResolutionStatement();
		instance._unresolved = unresolved;
		instance._resolutionEntries = resolutionEntries;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._unresolved.serialize());
		buffer_.write(converter.intToBytes(this._resolutionEntries.length, 4, false)); /* bound: resolution_entries_count */ 
		arrayHelpers.write_array(buffer_, this._resolutionEntries);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `unresolved: ${this._unresolved.toString()}`;
		result += `resolutionEntries: ${this._resolutionEntries.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class MosaicResolutionEntry {
	static TYPE_HINTS = {
		source: 'struct:ReceiptSource',
		resolvedValue: 'pod:MosaicId'
	};

	constructor() {
		this._source = new ReceiptSource();
		this._resolvedValue = new MosaicId();
	}

	get source() {
		return this._source;
	}

	get resolvedValue() {
		return this._resolvedValue;
	}

	set source(value) {
		this._source = value;
	}

	set resolvedValue(value) {
		this._resolvedValue = value;
	}

	get size() {
		let size = 0
		size += this.source.size;
		size += this.resolvedValue.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const source = ReceiptSource.deserialize(view.buffer);
		view.shift(source.size);
		const resolvedValue = MosaicId.deserialize(view.buffer);
		view.shift(resolvedValue.size);

		const instance = new MosaicResolutionEntry();
		instance._source = source;
		instance._resolvedValue = resolvedValue;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._source.serialize());
		buffer_.write(this._resolvedValue.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `source: ${this._source.toString()}`;
		result += `resolvedValue: ${this._resolvedValue.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicResolutionStatement {
	static TYPE_HINTS = {
		unresolved: 'pod:UnresolvedMosaicId',
		resolutionEntries: 'array[MosaicResolutionEntry]'
	};

	constructor() {
		this._unresolved = new UnresolvedMosaicId();
		this._resolutionEntries = [];
	}

	get unresolved() {
		return this._unresolved;
	}

	get resolutionEntries() {
		return this._resolutionEntries;
	}

	set unresolved(value) {
		this._unresolved = value;
	}

	set resolutionEntries(value) {
		this._resolutionEntries = value;
	}

	get size() {
		let size = 0
		size += this.unresolved.size;
		size += 4;
		size += this.resolutionEntries.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const unresolved = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(unresolved.size);
		const resolutionEntriesCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const resolutionEntries = arrayHelpers.read_array_count(view.buffer, MosaicResolutionEntry, resolutionEntriesCount);
		view.shift(resolutionEntries.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new MosaicResolutionStatement();
		instance._unresolved = unresolved;
		instance._resolutionEntries = resolutionEntries;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._unresolved.serialize());
		buffer_.write(converter.intToBytes(this._resolutionEntries.length, 4, false)); /* bound: resolution_entries_count */ 
		arrayHelpers.write_array(buffer_, this._resolutionEntries);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `unresolved: ${this._unresolved.toString()}`;
		result += `resolutionEntries: ${this._resolutionEntries.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class TransactionStatement {
	static TYPE_HINTS = {
		receipts: 'array[Receipt]'
	};

	constructor() {
		this._primaryId = 0;
		this._secondaryId = 0;
		this._receipts = [];
	}

	get primaryId() {
		return this._primaryId;
	}

	get secondaryId() {
		return this._secondaryId;
	}

	get receipts() {
		return this._receipts;
	}

	set primaryId(value) {
		this._primaryId = value;
	}

	set secondaryId(value) {
		this._secondaryId = value;
	}

	set receipts(value) {
		this._receipts = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += 4;
		size += this.receipts.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const primaryId = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const secondaryId = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const receiptCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const receipts = arrayHelpers.read_array_count(view.buffer, Receipt, receiptCount);
		view.shift(receipts.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new TransactionStatement();
		instance._primaryId = primaryId;
		instance._secondaryId = secondaryId;
		instance._receipts = receipts;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._primaryId, 4, false));
		buffer_.write(converter.intToBytes(this._secondaryId, 4, false));
		buffer_.write(converter.intToBytes(this._receipts.length, 4, false)); /* bound: receipt_count */ 
		arrayHelpers.write_array(buffer_, this._receipts);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `primaryId: ${"0x".concat(this._primaryId.toString(16))}`;
		result += `secondaryId: ${"0x".concat(this._secondaryId.toString(16))}`;
		result += `receipts: ${this._receipts.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class BlockStatement {
	static TYPE_HINTS = {
		transactionStatements: 'array[TransactionStatement]',
		addressResolutionStatements: 'array[AddressResolutionStatement]',
		mosaicResolutionStatements: 'array[MosaicResolutionStatement]'
	};

	constructor() {
		this._transactionStatements = [];
		this._addressResolutionStatements = [];
		this._mosaicResolutionStatements = [];
	}

	get transactionStatements() {
		return this._transactionStatements;
	}

	get addressResolutionStatements() {
		return this._addressResolutionStatements;
	}

	get mosaicResolutionStatements() {
		return this._mosaicResolutionStatements;
	}

	set transactionStatements(value) {
		this._transactionStatements = value;
	}

	set addressResolutionStatements(value) {
		this._addressResolutionStatements = value;
	}

	set mosaicResolutionStatements(value) {
		this._mosaicResolutionStatements = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += this.transactionStatements.map(e => e.size).reduce((a, b) => a + b, 0);
		size += 4;
		size += this.addressResolutionStatements.map(e => e.size).reduce((a, b) => a + b, 0);
		size += 4;
		size += this.mosaicResolutionStatements.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const transactionStatementCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const transactionStatements = arrayHelpers.read_array_count(view.buffer, TransactionStatement, transactionStatementCount);
		view.shift(transactionStatements.map(e => e.size).reduce((a, b) => a + b, 0));
		const addressResolutionStatementCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const addressResolutionStatements = arrayHelpers.read_array_count(view.buffer, AddressResolutionStatement, addressResolutionStatementCount);
		view.shift(addressResolutionStatements.map(e => e.size).reduce((a, b) => a + b, 0));
		const mosaicResolutionStatementCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const mosaicResolutionStatements = arrayHelpers.read_array_count(view.buffer, MosaicResolutionStatement, mosaicResolutionStatementCount);
		view.shift(mosaicResolutionStatements.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new BlockStatement();
		instance._transactionStatements = transactionStatements;
		instance._addressResolutionStatements = addressResolutionStatements;
		instance._mosaicResolutionStatements = mosaicResolutionStatements;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._transactionStatements.length, 4, false)); /* bound: transaction_statement_count */ 
		arrayHelpers.write_array(buffer_, this._transactionStatements);
		buffer_.write(converter.intToBytes(this._addressResolutionStatements.length, 4, false)); /* bound: address_resolution_statement_count */ 
		arrayHelpers.write_array(buffer_, this._addressResolutionStatements);
		buffer_.write(converter.intToBytes(this._mosaicResolutionStatements.length, 4, false)); /* bound: mosaic_resolution_statement_count */ 
		arrayHelpers.write_array(buffer_, this._mosaicResolutionStatements);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `transactionStatements: ${this._transactionStatements.map(e => e.toString())}`;
		result += `addressResolutionStatements: ${this._addressResolutionStatements.map(e => e.toString())}`;
		result += `mosaicResolutionStatements: ${this._mosaicResolutionStatements.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountType {
	static UNLINKED = new AccountType(0);

	static MAIN = new AccountType(1);

	static REMOTE = new AccountType(2);

	static REMOTE_UNLINKED = new AccountType(3);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1, 2, 3
		];
		const keys = [
			'UNLINKED', 'MAIN', 'REMOTE', 'REMOTE_UNLINKED'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return AccountType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class AccountKeyTypeFlags {
	static UNSET = new AccountKeyTypeFlags(0);

	static LINKED = new AccountKeyTypeFlags(1);

	static NODE = new AccountKeyTypeFlags(2);

	static VRF = new AccountKeyTypeFlags(4);

	constructor(value) {
		this.value = value;
	}

	has(flag) {
		return 0 !== (this.value & flag);
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new AccountKeyTypeFlags(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class AccountStateFormat {
	static REGULAR = new AccountStateFormat(0);

	static HIGH_VALUE = new AccountStateFormat(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'REGULAR', 'HIGH_VALUE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return AccountStateFormat[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class PinnedVotingKey {
	static TYPE_HINTS = {
		votingKey: 'pod:VotingPublicKey',
		startEpoch: 'pod:FinalizationEpoch',
		endEpoch: 'pod:FinalizationEpoch'
	};

	constructor() {
		this._votingKey = new VotingPublicKey();
		this._startEpoch = new FinalizationEpoch();
		this._endEpoch = new FinalizationEpoch();
	}

	get votingKey() {
		return this._votingKey;
	}

	get startEpoch() {
		return this._startEpoch;
	}

	get endEpoch() {
		return this._endEpoch;
	}

	set votingKey(value) {
		this._votingKey = value;
	}

	set startEpoch(value) {
		this._startEpoch = value;
	}

	set endEpoch(value) {
		this._endEpoch = value;
	}

	get size() {
		let size = 0
		size += this.votingKey.size;
		size += this.startEpoch.size;
		size += this.endEpoch.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const votingKey = VotingPublicKey.deserialize(view.buffer);
		view.shift(votingKey.size);
		const startEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(endEpoch.size);

		const instance = new PinnedVotingKey();
		instance._votingKey = votingKey;
		instance._startEpoch = startEpoch;
		instance._endEpoch = endEpoch;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._votingKey.serialize());
		buffer_.write(this._startEpoch.serialize());
		buffer_.write(this._endEpoch.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `votingKey: ${this._votingKey.toString()}`;
		result += `startEpoch: ${this._startEpoch.toString()}`;
		result += `endEpoch: ${this._endEpoch.toString()}`;
		result += ')';
		return result;
	}
}

class ImportanceSnapshot {
	static TYPE_HINTS = {
		importance: 'pod:Importance',
		height: 'pod:ImportanceHeight'
	};

	constructor() {
		this._importance = new Importance();
		this._height = new ImportanceHeight();
	}

	get importance() {
		return this._importance;
	}

	get height() {
		return this._height;
	}

	set importance(value) {
		this._importance = value;
	}

	set height(value) {
		this._height = value;
	}

	get size() {
		let size = 0
		size += this.importance.size;
		size += this.height.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const importance = Importance.deserialize(view.buffer);
		view.shift(importance.size);
		const height = ImportanceHeight.deserialize(view.buffer);
		view.shift(height.size);

		const instance = new ImportanceSnapshot();
		instance._importance = importance;
		instance._height = height;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._importance.serialize());
		buffer_.write(this._height.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `importance: ${this._importance.toString()}`;
		result += `height: ${this._height.toString()}`;
		result += ')';
		return result;
	}
}

class HeightActivityBucket {
	static TYPE_HINTS = {
		startHeight: 'pod:ImportanceHeight',
		totalFeesPaid: 'pod:Amount'
	};

	constructor() {
		this._startHeight = new ImportanceHeight();
		this._totalFeesPaid = new Amount();
		this._beneficiaryCount = 0;
		this._rawScore = 0n;
	}

	get startHeight() {
		return this._startHeight;
	}

	get totalFeesPaid() {
		return this._totalFeesPaid;
	}

	get beneficiaryCount() {
		return this._beneficiaryCount;
	}

	get rawScore() {
		return this._rawScore;
	}

	set startHeight(value) {
		this._startHeight = value;
	}

	set totalFeesPaid(value) {
		this._totalFeesPaid = value;
	}

	set beneficiaryCount(value) {
		this._beneficiaryCount = value;
	}

	set rawScore(value) {
		this._rawScore = value;
	}

	get size() {
		let size = 0
		size += this.startHeight.size;
		size += this.totalFeesPaid.size;
		size += 4;
		size += 8;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const startHeight = ImportanceHeight.deserialize(view.buffer);
		view.shift(startHeight.size);
		const totalFeesPaid = Amount.deserialize(view.buffer);
		view.shift(totalFeesPaid.size);
		const beneficiaryCount = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const rawScore = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);

		const instance = new HeightActivityBucket();
		instance._startHeight = startHeight;
		instance._totalFeesPaid = totalFeesPaid;
		instance._beneficiaryCount = beneficiaryCount;
		instance._rawScore = rawScore;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._startHeight.serialize());
		buffer_.write(this._totalFeesPaid.serialize());
		buffer_.write(converter.intToBytes(this._beneficiaryCount, 4, false));
		buffer_.write(converter.intToBytes(this._rawScore, 8, false));
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `startHeight: ${this._startHeight.toString()}`;
		result += `totalFeesPaid: ${this._totalFeesPaid.toString()}`;
		result += `beneficiaryCount: ${"0x".concat(this._beneficiaryCount.toString(16))}`;
		result += `rawScore: ${"0x".concat(this._rawScore.toString(16))}`;
		result += ')';
		return result;
	}
}

class HeightActivityBuckets {
	static TYPE_HINTS = {
		buckets: 'array[HeightActivityBucket]'
	};

	constructor() {
		this._buckets = [];
	}

	get buckets() {
		return this._buckets;
	}

	set buckets(value) {
		this._buckets = value;
	}

	get size() {
		let size = 0
		size += this.buckets.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const buckets = arrayHelpers.read_array_count(view.buffer, HeightActivityBucket, 5);
		view.shift(buckets.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new HeightActivityBuckets();
		instance._buckets = buckets;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		arrayHelpers.write_array_count(buffer_, this._buckets, 5);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `buckets: ${this._buckets.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountState {
	static TYPE_HINTS = {
		address: 'pod:Address',
		addressHeight: 'pod:Height',
		publicKey: 'pod:PublicKey',
		publicKeyHeight: 'pod:Height',
		accountType: 'enum:AccountType',
		format: 'enum:AccountStateFormat',
		supplementalPublicKeysMask: 'enum:AccountKeyTypeFlags',
		linkedPublicKey: 'pod:PublicKey',
		nodePublicKey: 'pod:PublicKey',
		vrfPublicKey: 'pod:PublicKey',
		votingPublicKeys: 'array[PinnedVotingKey]',
		importanceSnapshots: 'struct:ImportanceSnapshot',
		activityBuckets: 'struct:HeightActivityBuckets',
		balances: 'array[Mosaic]'
	};

	constructor() {
		this._version = 0;
		this._address = new Address();
		this._addressHeight = new Height();
		this._publicKey = new PublicKey();
		this._publicKeyHeight = new Height();
		this._accountType = AccountType.UNLINKED;
		this._format = AccountStateFormat.REGULAR;
		this._supplementalPublicKeysMask = AccountKeyTypeFlags.UNSET;
		this._linkedPublicKey = new PublicKey();
		this._nodePublicKey = new PublicKey();
		this._vrfPublicKey = new PublicKey();
		this._votingPublicKeys = [];
		this._importanceSnapshots = new ImportanceSnapshot();
		this._activityBuckets = new HeightActivityBuckets();
		this._balances = [];
	}

	get version() {
		return this._version;
	}

	get address() {
		return this._address;
	}

	get addressHeight() {
		return this._addressHeight;
	}

	get publicKey() {
		return this._publicKey;
	}

	get publicKeyHeight() {
		return this._publicKeyHeight;
	}

	get accountType() {
		return this._accountType;
	}

	get format() {
		return this._format;
	}

	get supplementalPublicKeysMask() {
		return this._supplementalPublicKeysMask;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get nodePublicKey() {
		return this._nodePublicKey;
	}

	get vrfPublicKey() {
		return this._vrfPublicKey;
	}

	get votingPublicKeys() {
		return this._votingPublicKeys;
	}

	get importanceSnapshots() {
		return this._importanceSnapshots;
	}

	get activityBuckets() {
		return this._activityBuckets;
	}

	get balances() {
		return this._balances;
	}

	set version(value) {
		this._version = value;
	}

	set address(value) {
		this._address = value;
	}

	set addressHeight(value) {
		this._addressHeight = value;
	}

	set publicKey(value) {
		this._publicKey = value;
	}

	set publicKeyHeight(value) {
		this._publicKeyHeight = value;
	}

	set accountType(value) {
		this._accountType = value;
	}

	set format(value) {
		this._format = value;
	}

	set supplementalPublicKeysMask(value) {
		this._supplementalPublicKeysMask = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set nodePublicKey(value) {
		this._nodePublicKey = value;
	}

	set vrfPublicKey(value) {
		this._vrfPublicKey = value;
	}

	set votingPublicKeys(value) {
		this._votingPublicKeys = value;
	}

	set importanceSnapshots(value) {
		this._importanceSnapshots = value;
	}

	set activityBuckets(value) {
		this._activityBuckets = value;
	}

	set balances(value) {
		this._balances = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.address.size;
		size += this.addressHeight.size;
		size += this.publicKey.size;
		size += this.publicKeyHeight.size;
		size += this.accountType.size;
		size += this.format.size;
		size += this.supplementalPublicKeysMask.size;
		size += 1;
		if (this.supplementalPublicKeysMask.has(AccountKeyTypeFlags.LINKED)) {
			size += this.linkedPublicKey.size;
		}
		if (this.supplementalPublicKeysMask.has(AccountKeyTypeFlags.NODE)) {
			size += this.nodePublicKey.size;
		}
		if (this.supplementalPublicKeysMask.has(AccountKeyTypeFlags.VRF)) {
			size += this.vrfPublicKey.size;
		}
		size += this.votingPublicKeys.map(e => e.size).reduce((a, b) => a + b, 0);
		if (AccountStateFormat.HIGH_VALUE == this.format) {
			size += this.importanceSnapshots.size;
		}
		if (AccountStateFormat.HIGH_VALUE == this.format) {
			size += this.activityBuckets.size;
		}
		size += 2;
		size += this.balances.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const address = Address.deserialize(view.buffer);
		view.shift(address.size);
		const addressHeight = Height.deserialize(view.buffer);
		view.shift(addressHeight.size);
		const publicKey = PublicKey.deserialize(view.buffer);
		view.shift(publicKey.size);
		const publicKeyHeight = Height.deserialize(view.buffer);
		view.shift(publicKeyHeight.size);
		const accountType = AccountType.deserialize(view.buffer);
		view.shift(accountType.size);
		const format = AccountStateFormat.deserialize(view.buffer);
		view.shift(format.size);
		const supplementalPublicKeysMask = AccountKeyTypeFlags.deserialize(view.buffer);
		view.shift(supplementalPublicKeysMask.size);
		const votingPublicKeysCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		let linkedPublicKey = null
		if (supplementalPublicKeysMask.has(AccountKeyTypeFlags.LINKED)) {
			linkedPublicKey = PublicKey.deserialize(view.buffer);
			view.shift(linkedPublicKey.size);
		}
		let nodePublicKey = null
		if (supplementalPublicKeysMask.has(AccountKeyTypeFlags.NODE)) {
			nodePublicKey = PublicKey.deserialize(view.buffer);
			view.shift(nodePublicKey.size);
		}
		let vrfPublicKey = null
		if (supplementalPublicKeysMask.has(AccountKeyTypeFlags.VRF)) {
			vrfPublicKey = PublicKey.deserialize(view.buffer);
			view.shift(vrfPublicKey.size);
		}
		const votingPublicKeys = arrayHelpers.read_array_count(view.buffer, PinnedVotingKey, votingPublicKeysCount);
		view.shift(votingPublicKeys.map(e => e.size).reduce((a, b) => a + b, 0));
		let importanceSnapshots = null
		if (AccountStateFormat.HIGH_VALUE == format) {
			importanceSnapshots = ImportanceSnapshot.deserialize(view.buffer);
			view.shift(importanceSnapshots.size);
		}
		let activityBuckets = null
		if (AccountStateFormat.HIGH_VALUE == format) {
			activityBuckets = HeightActivityBuckets.deserialize(view.buffer);
			view.shift(activityBuckets.size);
		}
		const balancesCount = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const balances = arrayHelpers.read_array_count(view.buffer, Mosaic, balancesCount);
		view.shift(balances.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountState();
		instance._version = version;
		instance._address = address;
		instance._addressHeight = addressHeight;
		instance._publicKey = publicKey;
		instance._publicKeyHeight = publicKeyHeight;
		instance._accountType = accountType;
		instance._format = format;
		instance._supplementalPublicKeysMask = supplementalPublicKeysMask;
		instance._linkedPublicKey = linkedPublicKey;
		instance._nodePublicKey = nodePublicKey;
		instance._vrfPublicKey = vrfPublicKey;
		instance._votingPublicKeys = votingPublicKeys;
		instance._importanceSnapshots = importanceSnapshots;
		instance._activityBuckets = activityBuckets;
		instance._balances = balances;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._address.serialize());
		buffer_.write(this._addressHeight.serialize());
		buffer_.write(this._publicKey.serialize());
		buffer_.write(this._publicKeyHeight.serialize());
		buffer_.write(this._accountType.serialize());
		buffer_.write(this._format.serialize());
		buffer_.write(this._supplementalPublicKeysMask.serialize());
		buffer_.write(converter.intToBytes(this._votingPublicKeys.length, 1, false)); /* bound: voting_public_keys_count */ 
		if (this.supplementalPublicKeysMask.has(AccountKeyTypeFlags.LINKED)) {
			buffer_.write(this._linkedPublicKey.serialize());
		}
		if (this.supplementalPublicKeysMask.has(AccountKeyTypeFlags.NODE)) {
			buffer_.write(this._nodePublicKey.serialize());
		}
		if (this.supplementalPublicKeysMask.has(AccountKeyTypeFlags.VRF)) {
			buffer_.write(this._vrfPublicKey.serialize());
		}
		arrayHelpers.write_array(buffer_, this._votingPublicKeys);
		if (AccountStateFormat.HIGH_VALUE == this.format) {
			buffer_.write(this._importanceSnapshots.serialize());
		}
		if (AccountStateFormat.HIGH_VALUE == this.format) {
			buffer_.write(this._activityBuckets.serialize());
		}
		buffer_.write(converter.intToBytes(this._balances.length, 2, false)); /* bound: balances_count */ 
		arrayHelpers.write_array(buffer_, this._balances);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `address: ${this._address.toString()}`;
		result += `addressHeight: ${this._addressHeight.toString()}`;
		result += `publicKey: ${this._publicKey.toString()}`;
		result += `publicKeyHeight: ${this._publicKeyHeight.toString()}`;
		result += `accountType: ${this._accountType.toString()}`;
		result += `format: ${this._format.toString()}`;
		result += `supplementalPublicKeysMask: ${this._supplementalPublicKeysMask.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `nodePublicKey: ${this._nodePublicKey.toString()}`;
		result += `vrfPublicKey: ${this._vrfPublicKey.toString()}`;
		result += `votingPublicKeys: ${this._votingPublicKeys.map(e => e.toString())}`;
		result += `importanceSnapshots: ${this._importanceSnapshots.toString()}`;
		result += `activityBuckets: ${this._activityBuckets.toString()}`;
		result += `balances: ${this._balances.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class LockStatus {
	static UNUSED = new LockStatus(0);

	static USED = new LockStatus(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'UNUSED', 'USED'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return LockStatus[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class HashLockInfo {
	static TYPE_HINTS = {
		ownerAddress: 'pod:Address',
		mosaic: 'struct:Mosaic',
		endHeight: 'pod:Height',
		status: 'enum:LockStatus',
		hash: 'pod:Hash256'
	};

	constructor() {
		this._version = 0;
		this._ownerAddress = new Address();
		this._mosaic = new Mosaic();
		this._endHeight = new Height();
		this._status = LockStatus.UNUSED;
		this._hash = new Hash256();
	}

	get version() {
		return this._version;
	}

	get ownerAddress() {
		return this._ownerAddress;
	}

	get mosaic() {
		return this._mosaic;
	}

	get endHeight() {
		return this._endHeight;
	}

	get status() {
		return this._status;
	}

	get hash() {
		return this._hash;
	}

	set version(value) {
		this._version = value;
	}

	set ownerAddress(value) {
		this._ownerAddress = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set endHeight(value) {
		this._endHeight = value;
	}

	set status(value) {
		this._status = value;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.ownerAddress.size;
		size += this.mosaic.size;
		size += this.endHeight.size;
		size += this.status.size;
		size += this.hash.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const ownerAddress = Address.deserialize(view.buffer);
		view.shift(ownerAddress.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const endHeight = Height.deserialize(view.buffer);
		view.shift(endHeight.size);
		const status = LockStatus.deserialize(view.buffer);
		view.shift(status.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shift(hash.size);

		const instance = new HashLockInfo();
		instance._version = version;
		instance._ownerAddress = ownerAddress;
		instance._mosaic = mosaic;
		instance._endHeight = endHeight;
		instance._status = status;
		instance._hash = hash;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._ownerAddress.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._endHeight.serialize());
		buffer_.write(this._status.serialize());
		buffer_.write(this._hash.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `ownerAddress: ${this._ownerAddress.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `endHeight: ${this._endHeight.toString()}`;
		result += `status: ${this._status.toString()}`;
		result += `hash: ${this._hash.toString()}`;
		result += ')';
		return result;
	}
}

class ScopedMetadataKey extends BaseValue {
	static SIZE = 8;

	constructor(scopedMetadataKey = 0n) {
		super(ScopedMetadataKey.SIZE, scopedMetadataKey);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new ScopedMetadataKey(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class MetadataType {
	static ACCOUNT = new MetadataType(0);

	static MOSAIC = new MetadataType(1);

	static NAMESPACE = new MetadataType(2);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1, 2
		];
		const keys = [
			'ACCOUNT', 'MOSAIC', 'NAMESPACE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return MetadataType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class MetadataValue {
	static TYPE_HINTS = {
		data: 'bytes_array'
	};

	constructor() {
		this._data = new Uint8Array();
	}

	get data() {
		return this._data;
	}

	set data(value) {
		this._data = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this._data.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const data = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, size);
		view.shift(size);

		const instance = new MetadataValue();
		instance._data = data;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._data.length, 2, false)); /* bound: size */ 
		buffer_.write(this._data);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `data: ${this._data /* FIXME */}`;
		result += ')';
		return result;
	}
}

class MetadataEntry {
	static TYPE_HINTS = {
		sourceAddress: 'pod:Address',
		targetAddress: 'pod:Address',
		scopedMetadataKey: 'pod:ScopedMetadataKey',
		metadataType: 'enum:MetadataType',
		value: 'struct:MetadataValue'
	};

	constructor() {
		this._version = 0;
		this._sourceAddress = new Address();
		this._targetAddress = new Address();
		this._scopedMetadataKey = new ScopedMetadataKey();
		this._targetId = 0n;
		this._metadataType = MetadataType.ACCOUNT;
		this._value = new MetadataValue();
	}

	get version() {
		return this._version;
	}

	get sourceAddress() {
		return this._sourceAddress;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get targetId() {
		return this._targetId;
	}

	get metadataType() {
		return this._metadataType;
	}

	get value() {
		return this._value;
	}

	set version(value) {
		this._version = value;
	}

	set sourceAddress(value) {
		this._sourceAddress = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set targetId(value) {
		this._targetId = value;
	}

	set metadataType(value) {
		this._metadataType = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.sourceAddress.size;
		size += this.targetAddress.size;
		size += this.scopedMetadataKey.size;
		size += 8;
		size += this.metadataType.size;
		size += this.value.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const sourceAddress = Address.deserialize(view.buffer);
		view.shift(sourceAddress.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = ScopedMetadataKey.deserialize(view.buffer);
		view.shift(scopedMetadataKey.size);
		const targetId = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const metadataType = MetadataType.deserialize(view.buffer);
		view.shift(metadataType.size);
		const value = MetadataValue.deserialize(view.buffer);
		view.shift(value.size);

		const instance = new MetadataEntry();
		instance._version = version;
		instance._sourceAddress = sourceAddress;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._targetId = targetId;
		instance._metadataType = metadataType;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._sourceAddress.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(this._scopedMetadataKey.serialize());
		buffer_.write(converter.intToBytes(this._targetId, 8, false));
		buffer_.write(this._metadataType.serialize());
		buffer_.write(this._value.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `sourceAddress: ${this._sourceAddress.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${this._scopedMetadataKey.toString()}`;
		result += `targetId: ${"0x".concat(this._targetId.toString(16))}`;
		result += `metadataType: ${this._metadataType.toString()}`;
		result += `value: ${this._value.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicNonce extends BaseValue {
	static SIZE = 4;

	constructor(mosaicNonce = 0) {
		super(MosaicNonce.SIZE, mosaicNonce);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicNonce(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

class MosaicFlags {
	static NONE = new MosaicFlags(0);

	static SUPPLY_MUTABLE = new MosaicFlags(1);

	static TRANSFERABLE = new MosaicFlags(2);

	static RESTRICTABLE = new MosaicFlags(4);

	static REVOKABLE = new MosaicFlags(8);

	constructor(value) {
		this.value = value;
	}

	has(flag) {
		return 0 !== (this.value & flag);
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicFlags(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class MosaicSupplyChangeAction {
	static DECREASE = new MosaicSupplyChangeAction(0);

	static INCREASE = new MosaicSupplyChangeAction(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'DECREASE', 'INCREASE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return MosaicSupplyChangeAction[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class MosaicProperties {
	static TYPE_HINTS = {
		flags: 'enum:MosaicFlags',
		duration: 'pod:BlockDuration'
	};

	constructor() {
		this._flags = MosaicFlags.NONE;
		this._divisibility = 0;
		this._duration = new BlockDuration();
	}

	get flags() {
		return this._flags;
	}

	get divisibility() {
		return this._divisibility;
	}

	get duration() {
		return this._duration;
	}

	set flags(value) {
		this._flags = value;
	}

	set divisibility(value) {
		this._divisibility = value;
	}

	set duration(value) {
		this._duration = value;
	}

	get size() {
		let size = 0
		size += this.flags.size;
		size += 1;
		size += this.duration.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const flags = MosaicFlags.deserialize(view.buffer);
		view.shift(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);

		const instance = new MosaicProperties();
		instance._flags = flags;
		instance._divisibility = divisibility;
		instance._duration = duration;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._flags.serialize());
		buffer_.write(converter.intToBytes(this._divisibility, 1, false));
		buffer_.write(this._duration.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `flags: ${this._flags.toString()}`;
		result += `divisibility: ${"0x".concat(this._divisibility.toString(16))}`;
		result += `duration: ${this._duration.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicDefinition {
	static TYPE_HINTS = {
		startHeight: 'pod:Height',
		ownerAddress: 'pod:Address',
		properties: 'struct:MosaicProperties'
	};

	constructor() {
		this._startHeight = new Height();
		this._ownerAddress = new Address();
		this._revision = 0;
		this._properties = new MosaicProperties();
	}

	get startHeight() {
		return this._startHeight;
	}

	get ownerAddress() {
		return this._ownerAddress;
	}

	get revision() {
		return this._revision;
	}

	get properties() {
		return this._properties;
	}

	set startHeight(value) {
		this._startHeight = value;
	}

	set ownerAddress(value) {
		this._ownerAddress = value;
	}

	set revision(value) {
		this._revision = value;
	}

	set properties(value) {
		this._properties = value;
	}

	get size() {
		let size = 0
		size += this.startHeight.size;
		size += this.ownerAddress.size;
		size += 4;
		size += this.properties.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const startHeight = Height.deserialize(view.buffer);
		view.shift(startHeight.size);
		const ownerAddress = Address.deserialize(view.buffer);
		view.shift(ownerAddress.size);
		const revision = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const properties = MosaicProperties.deserialize(view.buffer);
		view.shift(properties.size);

		const instance = new MosaicDefinition();
		instance._startHeight = startHeight;
		instance._ownerAddress = ownerAddress;
		instance._revision = revision;
		instance._properties = properties;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._startHeight.serialize());
		buffer_.write(this._ownerAddress.serialize());
		buffer_.write(converter.intToBytes(this._revision, 4, false));
		buffer_.write(this._properties.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `startHeight: ${this._startHeight.toString()}`;
		result += `ownerAddress: ${this._ownerAddress.toString()}`;
		result += `revision: ${"0x".concat(this._revision.toString(16))}`;
		result += `properties: ${this._properties.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicEntry {
	static TYPE_HINTS = {
		mosaicId: 'pod:MosaicId',
		supply: 'pod:Amount',
		definition: 'struct:MosaicDefinition'
	};

	constructor() {
		this._version = 0;
		this._mosaicId = new MosaicId();
		this._supply = new Amount();
		this._definition = new MosaicDefinition();
	}

	get version() {
		return this._version;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get supply() {
		return this._supply;
	}

	get definition() {
		return this._definition;
	}

	set version(value) {
		this._version = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set supply(value) {
		this._supply = value;
	}

	set definition(value) {
		this._definition = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.mosaicId.size;
		size += this.supply.size;
		size += this.definition.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const supply = Amount.deserialize(view.buffer);
		view.shift(supply.size);
		const definition = MosaicDefinition.deserialize(view.buffer);
		view.shift(definition.size);

		const instance = new MosaicEntry();
		instance._version = version;
		instance._mosaicId = mosaicId;
		instance._supply = supply;
		instance._definition = definition;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._supply.serialize());
		buffer_.write(this._definition.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `supply: ${this._supply.toString()}`;
		result += `definition: ${this._definition.toString()}`;
		result += ')';
		return result;
	}
}

class MultisigEntry {
	static TYPE_HINTS = {
		accountAddress: 'pod:Address',
		cosignatoryAddresses: 'array[Address]',
		multisigAddresses: 'array[Address]'
	};

	constructor() {
		this._version = 0;
		this._minApproval = 0;
		this._minRemoval = 0;
		this._accountAddress = new Address();
		this._cosignatoryAddresses = [];
		this._multisigAddresses = [];
	}

	get version() {
		return this._version;
	}

	get minApproval() {
		return this._minApproval;
	}

	get minRemoval() {
		return this._minRemoval;
	}

	get accountAddress() {
		return this._accountAddress;
	}

	get cosignatoryAddresses() {
		return this._cosignatoryAddresses;
	}

	get multisigAddresses() {
		return this._multisigAddresses;
	}

	set version(value) {
		this._version = value;
	}

	set minApproval(value) {
		this._minApproval = value;
	}

	set minRemoval(value) {
		this._minRemoval = value;
	}

	set accountAddress(value) {
		this._accountAddress = value;
	}

	set cosignatoryAddresses(value) {
		this._cosignatoryAddresses = value;
	}

	set multisigAddresses(value) {
		this._multisigAddresses = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += 4;
		size += 4;
		size += this.accountAddress.size;
		size += 8;
		size += this.cosignatoryAddresses.map(e => e.size).reduce((a, b) => a + b, 0);
		size += 8;
		size += this.multisigAddresses.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const minApproval = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const minRemoval = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const accountAddress = Address.deserialize(view.buffer);
		view.shift(accountAddress.size);
		const cosignatoryAddressesCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const cosignatoryAddresses = arrayHelpers.read_array_count(view.buffer, Address, cosignatoryAddressesCount);
		view.shift(cosignatoryAddresses.map(e => e.size).reduce((a, b) => a + b, 0));
		const multisigAddressesCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const multisigAddresses = arrayHelpers.read_array_count(view.buffer, Address, multisigAddressesCount);
		view.shift(multisigAddresses.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new MultisigEntry();
		instance._version = version;
		instance._minApproval = minApproval;
		instance._minRemoval = minRemoval;
		instance._accountAddress = accountAddress;
		instance._cosignatoryAddresses = cosignatoryAddresses;
		instance._multisigAddresses = multisigAddresses;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(converter.intToBytes(this._minApproval, 4, false));
		buffer_.write(converter.intToBytes(this._minRemoval, 4, false));
		buffer_.write(this._accountAddress.serialize());
		buffer_.write(converter.intToBytes(this._cosignatoryAddresses.length, 8, false)); /* bound: cosignatory_addresses_count */ 
		arrayHelpers.write_array(buffer_, this._cosignatoryAddresses);
		buffer_.write(converter.intToBytes(this._multisigAddresses.length, 8, false)); /* bound: multisig_addresses_count */ 
		arrayHelpers.write_array(buffer_, this._multisigAddresses);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `minApproval: ${"0x".concat(this._minApproval.toString(16))}`;
		result += `minRemoval: ${"0x".concat(this._minRemoval.toString(16))}`;
		result += `accountAddress: ${this._accountAddress.toString()}`;
		result += `cosignatoryAddresses: ${this._cosignatoryAddresses.map(e => e.toString())}`;
		result += `multisigAddresses: ${this._multisigAddresses.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class NamespaceLifetime {
	static TYPE_HINTS = {
		lifetimeStart: 'pod:Height',
		lifetimeEnd: 'pod:Height'
	};

	constructor() {
		this._lifetimeStart = new Height();
		this._lifetimeEnd = new Height();
	}

	get lifetimeStart() {
		return this._lifetimeStart;
	}

	get lifetimeEnd() {
		return this._lifetimeEnd;
	}

	set lifetimeStart(value) {
		this._lifetimeStart = value;
	}

	set lifetimeEnd(value) {
		this._lifetimeEnd = value;
	}

	get size() {
		let size = 0
		size += this.lifetimeStart.size;
		size += this.lifetimeEnd.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const lifetimeStart = Height.deserialize(view.buffer);
		view.shift(lifetimeStart.size);
		const lifetimeEnd = Height.deserialize(view.buffer);
		view.shift(lifetimeEnd.size);

		const instance = new NamespaceLifetime();
		instance._lifetimeStart = lifetimeStart;
		instance._lifetimeEnd = lifetimeEnd;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._lifetimeStart.serialize());
		buffer_.write(this._lifetimeEnd.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `lifetimeStart: ${this._lifetimeStart.toString()}`;
		result += `lifetimeEnd: ${this._lifetimeEnd.toString()}`;
		result += ')';
		return result;
	}
}

class NamespaceAliasType {
	static NONE = new NamespaceAliasType(0);

	static MOSAIC_ID = new NamespaceAliasType(1);

	static ADDRESS = new NamespaceAliasType(2);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1, 2
		];
		const keys = [
			'NONE', 'MOSAIC_ID', 'ADDRESS'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return NamespaceAliasType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class NamespaceAlias {
	static TYPE_HINTS = {
		namespaceAliasType: 'enum:NamespaceAliasType',
		mosaicAlias: 'pod:MosaicId',
		addressAlias: 'pod:Address'
	};

	constructor() {
		this._namespaceAliasType = NamespaceAliasType.NONE;
		this._mosaicAlias = new MosaicId();
		this._addressAlias = new Address();
	}

	get namespaceAliasType() {
		return this._namespaceAliasType;
	}

	get mosaicAlias() {
		return this._mosaicAlias;
	}

	get addressAlias() {
		return this._addressAlias;
	}

	set namespaceAliasType(value) {
		this._namespaceAliasType = value;
	}

	set mosaicAlias(value) {
		this._mosaicAlias = value;
	}

	set addressAlias(value) {
		this._addressAlias = value;
	}

	get size() {
		let size = 0
		size += this.namespaceAliasType.size;
		if (NamespaceAliasType.MOSAIC_ID == this.namespaceAliasType) {
			size += this.mosaicAlias.size;
		}
		if (NamespaceAliasType.ADDRESS == this.namespaceAliasType) {
			size += this.addressAlias.size;
		}
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const namespaceAliasType = NamespaceAliasType.deserialize(view.buffer);
		view.shift(namespaceAliasType.size);
		let mosaicAlias = null
		if (NamespaceAliasType.MOSAIC_ID == namespaceAliasType) {
			mosaicAlias = MosaicId.deserialize(view.buffer);
			view.shift(mosaicAlias.size);
		}
		let addressAlias = null
		if (NamespaceAliasType.ADDRESS == namespaceAliasType) {
			addressAlias = Address.deserialize(view.buffer);
			view.shift(addressAlias.size);
		}

		const instance = new NamespaceAlias();
		instance._namespaceAliasType = namespaceAliasType;
		instance._mosaicAlias = mosaicAlias;
		instance._addressAlias = addressAlias;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._namespaceAliasType.serialize());
		if (NamespaceAliasType.MOSAIC_ID == this.namespaceAliasType) {
			buffer_.write(this._mosaicAlias.serialize());
		}
		if (NamespaceAliasType.ADDRESS == this.namespaceAliasType) {
			buffer_.write(this._addressAlias.serialize());
		}
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `namespaceAliasType: ${this._namespaceAliasType.toString()}`;
		result += `mosaicAlias: ${this._mosaicAlias.toString()}`;
		result += `addressAlias: ${this._addressAlias.toString()}`;
		result += ')';
		return result;
	}
}

class NamespacePath {
	static TYPE_HINTS = {
		path: 'array[NamespaceId]',
		alias: 'struct:NamespaceAlias'
	};

	constructor() {
		this._path = [];
		this._alias = new NamespaceAlias();
	}

	get path() {
		return this._path;
	}

	get alias() {
		return this._alias;
	}

	set path(value) {
		this._path = value;
	}

	set alias(value) {
		this._alias = value;
	}

	get size() {
		let size = 0
		size += 1;
		size += this.path.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.alias.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const pathSize = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const path = arrayHelpers.read_array_count(view.buffer, NamespaceId, pathSize);
		view.shift(path.map(e => e.size).reduce((a, b) => a + b, 0));
		const alias = NamespaceAlias.deserialize(view.buffer);
		view.shift(alias.size);

		const instance = new NamespacePath();
		instance._path = path;
		instance._alias = alias;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._path.length, 1, false)); /* bound: path_size */ 
		arrayHelpers.write_array(buffer_, this._path);
		buffer_.write(this._alias.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `path: ${this._path.map(e => e.toString())}`;
		result += `alias: ${this._alias.toString()}`;
		result += ')';
		return result;
	}
}

class RootNamespaceHistory {
	static TYPE_HINTS = {
		id: 'pod:NamespaceId',
		ownerAddress: 'pod:Address',
		lifetime: 'struct:NamespaceLifetime',
		rootAlias: 'struct:NamespaceAlias',
		paths: 'array[NamespacePath]'
	};

	constructor() {
		this._version = 0;
		this._id = new NamespaceId();
		this._ownerAddress = new Address();
		this._lifetime = new NamespaceLifetime();
		this._rootAlias = new NamespaceAlias();
		this._paths = [];
	}

	get version() {
		return this._version;
	}

	get id() {
		return this._id;
	}

	get ownerAddress() {
		return this._ownerAddress;
	}

	get lifetime() {
		return this._lifetime;
	}

	get rootAlias() {
		return this._rootAlias;
	}

	get paths() {
		return this._paths;
	}

	set version(value) {
		this._version = value;
	}

	set id(value) {
		this._id = value;
	}

	set ownerAddress(value) {
		this._ownerAddress = value;
	}

	set lifetime(value) {
		this._lifetime = value;
	}

	set rootAlias(value) {
		this._rootAlias = value;
	}

	set paths(value) {
		this._paths = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.id.size;
		size += this.ownerAddress.size;
		size += this.lifetime.size;
		size += this.rootAlias.size;
		size += 8;
		size += this.paths.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const id = NamespaceId.deserialize(view.buffer);
		view.shift(id.size);
		const ownerAddress = Address.deserialize(view.buffer);
		view.shift(ownerAddress.size);
		const lifetime = NamespaceLifetime.deserialize(view.buffer);
		view.shift(lifetime.size);
		const rootAlias = NamespaceAlias.deserialize(view.buffer);
		view.shift(rootAlias.size);
		const childrenCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const paths = arrayHelpers.read_array_count(view.buffer, NamespacePath, childrenCount, e => e.path.value);
		view.shift(paths.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new RootNamespaceHistory();
		instance._version = version;
		instance._id = id;
		instance._ownerAddress = ownerAddress;
		instance._lifetime = lifetime;
		instance._rootAlias = rootAlias;
		instance._paths = paths;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._id.serialize());
		buffer_.write(this._ownerAddress.serialize());
		buffer_.write(this._lifetime.serialize());
		buffer_.write(this._rootAlias.serialize());
		buffer_.write(converter.intToBytes(this._paths.length, 8, false)); /* bound: children_count */ 
		arrayHelpers.write_array(buffer_, this._paths, e => e.path.value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `id: ${this._id.toString()}`;
		result += `ownerAddress: ${this._ownerAddress.toString()}`;
		result += `lifetime: ${this._lifetime.toString()}`;
		result += `rootAlias: ${this._rootAlias.toString()}`;
		result += `paths: ${this._paths.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountRestrictionFlags {
	static ADDRESS = new AccountRestrictionFlags(1);

	static MOSAIC_ID = new AccountRestrictionFlags(2);

	static TRANSACTION_TYPE = new AccountRestrictionFlags(4);

	static OUTGOING = new AccountRestrictionFlags(16384);

	static BLOCK = new AccountRestrictionFlags(32768);

	constructor(value) {
		this.value = value;
	}

	has(flag) {
		return 0 !== (this.value & flag);
	}

	get size() {
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new AccountRestrictionFlags(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}
}

class AccountRestrictionAddressValue {
	static TYPE_HINTS = {
		restrictionValues: 'array[Address]'
	};

	constructor() {
		this._restrictionValues = [];
	}

	get restrictionValues() {
		return this._restrictionValues;
	}

	set restrictionValues(value) {
		this._restrictionValues = value;
	}

	get size() {
		let size = 0
		size += 8;
		size += this.restrictionValues.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const restrictionValuesCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const restrictionValues = arrayHelpers.read_array_count(view.buffer, Address, restrictionValuesCount);
		view.shift(restrictionValues.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountRestrictionAddressValue();
		instance._restrictionValues = restrictionValues;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._restrictionValues.length, 8, false)); /* bound: restriction_values_count */ 
		arrayHelpers.write_array(buffer_, this._restrictionValues);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `restrictionValues: ${this._restrictionValues.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountRestrictionMosaicValue {
	static TYPE_HINTS = {
		restrictionValues: 'array[MosaicId]'
	};

	constructor() {
		this._restrictionValues = [];
	}

	get restrictionValues() {
		return this._restrictionValues;
	}

	set restrictionValues(value) {
		this._restrictionValues = value;
	}

	get size() {
		let size = 0
		size += 8;
		size += this.restrictionValues.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const restrictionValuesCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const restrictionValues = arrayHelpers.read_array_count(view.buffer, MosaicId, restrictionValuesCount);
		view.shift(restrictionValues.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountRestrictionMosaicValue();
		instance._restrictionValues = restrictionValues;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._restrictionValues.length, 8, false)); /* bound: restriction_values_count */ 
		arrayHelpers.write_array(buffer_, this._restrictionValues);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `restrictionValues: ${this._restrictionValues.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountRestrictionTransactionTypeValue {
	static TYPE_HINTS = {
		restrictionValues: 'array[TransactionType]'
	};

	constructor() {
		this._restrictionValues = [];
	}

	get restrictionValues() {
		return this._restrictionValues;
	}

	set restrictionValues(value) {
		this._restrictionValues = value;
	}

	get size() {
		let size = 0
		size += 8;
		size += this.restrictionValues.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const restrictionValuesCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const restrictionValues = arrayHelpers.read_array_count(view.buffer, TransactionType, restrictionValuesCount);
		view.shift(restrictionValues.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountRestrictionTransactionTypeValue();
		instance._restrictionValues = restrictionValues;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._restrictionValues.length, 8, false)); /* bound: restriction_values_count */ 
		arrayHelpers.write_array(buffer_, this._restrictionValues);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `restrictionValues: ${this._restrictionValues.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountRestrictionsInfo {
	static TYPE_HINTS = {
		restrictionFlags: 'enum:AccountRestrictionFlags',
		addressRestrictions: 'struct:AccountRestrictionAddressValue',
		mosaicIdRestrictions: 'struct:AccountRestrictionMosaicValue',
		transactionTypeRestrictions: 'struct:AccountRestrictionTransactionTypeValue'
	};

	constructor() {
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._addressRestrictions = new AccountRestrictionAddressValue();
		this._mosaicIdRestrictions = new AccountRestrictionMosaicValue();
		this._transactionTypeRestrictions = new AccountRestrictionTransactionTypeValue();
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get addressRestrictions() {
		return this._addressRestrictions;
	}

	get mosaicIdRestrictions() {
		return this._mosaicIdRestrictions;
	}

	get transactionTypeRestrictions() {
		return this._transactionTypeRestrictions;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set addressRestrictions(value) {
		this._addressRestrictions = value;
	}

	set mosaicIdRestrictions(value) {
		this._mosaicIdRestrictions = value;
	}

	set transactionTypeRestrictions(value) {
		this._transactionTypeRestrictions = value;
	}

	get size() {
		let size = 0
		size += this.restrictionFlags.size;
		if (this.restrictionFlags.has(AccountRestrictionFlags.ADDRESS)) {
			size += this.addressRestrictions.size;
		}
		if (this.restrictionFlags.has(AccountRestrictionFlags.MOSAIC_ID)) {
			size += this.mosaicIdRestrictions.size;
		}
		if (this.restrictionFlags.has(AccountRestrictionFlags.TRANSACTION_TYPE)) {
			size += this.transactionTypeRestrictions.size;
		}
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		let addressRestrictions = null
		if (restrictionFlags.has(AccountRestrictionFlags.ADDRESS)) {
			addressRestrictions = AccountRestrictionAddressValue.deserialize(view.buffer);
			view.shift(addressRestrictions.size);
		}
		let mosaicIdRestrictions = null
		if (restrictionFlags.has(AccountRestrictionFlags.MOSAIC_ID)) {
			mosaicIdRestrictions = AccountRestrictionMosaicValue.deserialize(view.buffer);
			view.shift(mosaicIdRestrictions.size);
		}
		let transactionTypeRestrictions = null
		if (restrictionFlags.has(AccountRestrictionFlags.TRANSACTION_TYPE)) {
			transactionTypeRestrictions = AccountRestrictionTransactionTypeValue.deserialize(view.buffer);
			view.shift(transactionTypeRestrictions.size);
		}

		const instance = new AccountRestrictionsInfo();
		instance._restrictionFlags = restrictionFlags;
		instance._addressRestrictions = addressRestrictions;
		instance._mosaicIdRestrictions = mosaicIdRestrictions;
		instance._transactionTypeRestrictions = transactionTypeRestrictions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._restrictionFlags.serialize());
		if (this.restrictionFlags.has(AccountRestrictionFlags.ADDRESS)) {
			buffer_.write(this._addressRestrictions.serialize());
		}
		if (this.restrictionFlags.has(AccountRestrictionFlags.MOSAIC_ID)) {
			buffer_.write(this._mosaicIdRestrictions.serialize());
		}
		if (this.restrictionFlags.has(AccountRestrictionFlags.TRANSACTION_TYPE)) {
			buffer_.write(this._transactionTypeRestrictions.serialize());
		}
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `addressRestrictions: ${this._addressRestrictions.toString()}`;
		result += `mosaicIdRestrictions: ${this._mosaicIdRestrictions.toString()}`;
		result += `transactionTypeRestrictions: ${this._transactionTypeRestrictions.toString()}`;
		result += ')';
		return result;
	}
}

class AccountRestrictions {
	static TYPE_HINTS = {
		address: 'pod:Address',
		restrictions: 'array[AccountRestrictionsInfo]'
	};

	constructor() {
		this._version = 0;
		this._address = new Address();
		this._restrictions = [];
	}

	get version() {
		return this._version;
	}

	get address() {
		return this._address;
	}

	get restrictions() {
		return this._restrictions;
	}

	set version(value) {
		this._version = value;
	}

	set address(value) {
		this._address = value;
	}

	set restrictions(value) {
		this._restrictions = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.address.size;
		size += 8;
		size += this.restrictions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const address = Address.deserialize(view.buffer);
		view.shift(address.size);
		const restrictionsCount = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const restrictions = arrayHelpers.read_array_count(view.buffer, AccountRestrictionsInfo, restrictionsCount);
		view.shift(restrictions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountRestrictions();
		instance._version = version;
		instance._address = address;
		instance._restrictions = restrictions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._address.serialize());
		buffer_.write(converter.intToBytes(this._restrictions.length, 8, false)); /* bound: restrictions_count */ 
		arrayHelpers.write_array(buffer_, this._restrictions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `address: ${this._address.toString()}`;
		result += `restrictions: ${this._restrictions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class MosaicRestrictionKey extends BaseValue {
	static SIZE = 8;

	constructor(mosaicRestrictionKey = 0n) {
		super(MosaicRestrictionKey.SIZE, mosaicRestrictionKey);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicRestrictionKey(converter.bytesToInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

class MosaicRestrictionType {
	static NONE = new MosaicRestrictionType(0);

	static EQ = new MosaicRestrictionType(1);

	static NE = new MosaicRestrictionType(2);

	static LT = new MosaicRestrictionType(3);

	static LE = new MosaicRestrictionType(4);

	static GT = new MosaicRestrictionType(5);

	static GE = new MosaicRestrictionType(6);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1, 2, 3, 4, 5, 6
		];
		const keys = [
			'NONE', 'EQ', 'NE', 'LT', 'LE', 'GT', 'GE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return MosaicRestrictionType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class MosaicRestrictionEntryType {
	static ADDRESS = new MosaicRestrictionEntryType(0);

	static GLOBAL = new MosaicRestrictionEntryType(1);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1
		];
		const keys = [
			'ADDRESS', 'GLOBAL'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return MosaicRestrictionEntryType[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class AddressKeyValue {
	static TYPE_HINTS = {
		key: 'pod:MosaicRestrictionKey'
	};

	constructor() {
		this._key = new MosaicRestrictionKey();
		this._value = 0n;
	}

	get key() {
		return this._key;
	}

	get value() {
		return this._value;
	}

	set key(value) {
		this._key = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += this.key.size;
		size += 8;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const key = MosaicRestrictionKey.deserialize(view.buffer);
		view.shift(key.size);
		const value = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);

		const instance = new AddressKeyValue();
		instance._key = key;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._key.serialize());
		buffer_.write(converter.intToBytes(this._value, 8, false));
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `key: ${this._key.toString()}`;
		result += `value: ${"0x".concat(this._value.toString(16))}`;
		result += ')';
		return result;
	}
}

class AddressKeyValueSet {
	static TYPE_HINTS = {
		keys: 'array[AddressKeyValue]'
	};

	constructor() {
		this._keys = [];
	}

	get keys() {
		return this._keys;
	}

	set keys(value) {
		this._keys = value;
	}

	get size() {
		let size = 0
		size += 1;
		size += this.keys.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const keyValueCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const keys = arrayHelpers.read_array_count(view.buffer, AddressKeyValue, keyValueCount, e => e.key.value);
		view.shift(keys.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AddressKeyValueSet();
		instance._keys = keys;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._keys.length, 1, false)); /* bound: key_value_count */ 
		arrayHelpers.write_array(buffer_, this._keys, e => e.key.value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `keys: ${this._keys.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class RestrictionRule {
	static TYPE_HINTS = {
		referenceMosaicId: 'pod:MosaicId',
		restrictionType: 'enum:MosaicRestrictionType'
	};

	constructor() {
		this._referenceMosaicId = new MosaicId();
		this._restrictionValue = 0n;
		this._restrictionType = MosaicRestrictionType.NONE;
	}

	get referenceMosaicId() {
		return this._referenceMosaicId;
	}

	get restrictionValue() {
		return this._restrictionValue;
	}

	get restrictionType() {
		return this._restrictionType;
	}

	set referenceMosaicId(value) {
		this._referenceMosaicId = value;
	}

	set restrictionValue(value) {
		this._restrictionValue = value;
	}

	set restrictionType(value) {
		this._restrictionType = value;
	}

	get size() {
		let size = 0
		size += this.referenceMosaicId.size;
		size += 8;
		size += this.restrictionType.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const referenceMosaicId = MosaicId.deserialize(view.buffer);
		view.shift(referenceMosaicId.size);
		const restrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const restrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shift(restrictionType.size);

		const instance = new RestrictionRule();
		instance._referenceMosaicId = referenceMosaicId;
		instance._restrictionValue = restrictionValue;
		instance._restrictionType = restrictionType;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._referenceMosaicId.serialize());
		buffer_.write(converter.intToBytes(this._restrictionValue, 8, false));
		buffer_.write(this._restrictionType.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `referenceMosaicId: ${this._referenceMosaicId.toString()}`;
		result += `restrictionValue: ${"0x".concat(this._restrictionValue.toString(16))}`;
		result += `restrictionType: ${this._restrictionType.toString()}`;
		result += ')';
		return result;
	}
}

class GlobalKeyValue {
	static TYPE_HINTS = {
		key: 'pod:MosaicRestrictionKey',
		restrictionRule: 'struct:RestrictionRule'
	};

	constructor() {
		this._key = new MosaicRestrictionKey();
		this._restrictionRule = new RestrictionRule();
	}

	get key() {
		return this._key;
	}

	get restrictionRule() {
		return this._restrictionRule;
	}

	set key(value) {
		this._key = value;
	}

	set restrictionRule(value) {
		this._restrictionRule = value;
	}

	get size() {
		let size = 0
		size += this.key.size;
		size += this.restrictionRule.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const key = MosaicRestrictionKey.deserialize(view.buffer);
		view.shift(key.size);
		const restrictionRule = RestrictionRule.deserialize(view.buffer);
		view.shift(restrictionRule.size);

		const instance = new GlobalKeyValue();
		instance._key = key;
		instance._restrictionRule = restrictionRule;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._key.serialize());
		buffer_.write(this._restrictionRule.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `key: ${this._key.toString()}`;
		result += `restrictionRule: ${this._restrictionRule.toString()}`;
		result += ')';
		return result;
	}
}

class GlobalKeyValueSet {
	static TYPE_HINTS = {
		keys: 'array[GlobalKeyValue]'
	};

	constructor() {
		this._keys = [];
	}

	get keys() {
		return this._keys;
	}

	set keys(value) {
		this._keys = value;
	}

	get size() {
		let size = 0
		size += 1;
		size += this.keys.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const keyValueCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const keys = arrayHelpers.read_array_count(view.buffer, GlobalKeyValue, keyValueCount, e => e.key.value);
		view.shift(keys.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new GlobalKeyValueSet();
		instance._keys = keys;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._keys.length, 1, false)); /* bound: key_value_count */ 
		arrayHelpers.write_array(buffer_, this._keys, e => e.key.value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `keys: ${this._keys.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class MosaicAddressRestrictionEntry {
	static TYPE_HINTS = {
		mosaicId: 'pod:MosaicId',
		address: 'pod:Address',
		keyPairs: 'struct:AddressKeyValueSet'
	};

	constructor() {
		this._mosaicId = new MosaicId();
		this._address = new Address();
		this._keyPairs = new AddressKeyValueSet();
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get address() {
		return this._address;
	}

	get keyPairs() {
		return this._keyPairs;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set address(value) {
		this._address = value;
	}

	set keyPairs(value) {
		this._keyPairs = value;
	}

	get size() {
		let size = 0
		size += this.mosaicId.size;
		size += this.address.size;
		size += this.keyPairs.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const address = Address.deserialize(view.buffer);
		view.shift(address.size);
		const keyPairs = AddressKeyValueSet.deserialize(view.buffer);
		view.shift(keyPairs.size);

		const instance = new MosaicAddressRestrictionEntry();
		instance._mosaicId = mosaicId;
		instance._address = address;
		instance._keyPairs = keyPairs;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._address.serialize());
		buffer_.write(this._keyPairs.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `address: ${this._address.toString()}`;
		result += `keyPairs: ${this._keyPairs.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicGlobalRestrictionEntry {
	static TYPE_HINTS = {
		mosaicId: 'pod:MosaicId',
		keyPairs: 'struct:GlobalKeyValueSet'
	};

	constructor() {
		this._mosaicId = new MosaicId();
		this._keyPairs = new GlobalKeyValueSet();
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get keyPairs() {
		return this._keyPairs;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set keyPairs(value) {
		this._keyPairs = value;
	}

	get size() {
		let size = 0
		size += this.mosaicId.size;
		size += this.keyPairs.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const keyPairs = GlobalKeyValueSet.deserialize(view.buffer);
		view.shift(keyPairs.size);

		const instance = new MosaicGlobalRestrictionEntry();
		instance._mosaicId = mosaicId;
		instance._keyPairs = keyPairs;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._keyPairs.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `keyPairs: ${this._keyPairs.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicRestrictionEntry {
	static TYPE_HINTS = {
		entryType: 'enum:MosaicRestrictionEntryType',
		addressEntry: 'struct:MosaicAddressRestrictionEntry',
		globalEntry: 'struct:MosaicGlobalRestrictionEntry'
	};

	constructor() {
		this._version = 0;
		this._entryType = MosaicRestrictionEntryType.ADDRESS;
		this._addressEntry = new MosaicAddressRestrictionEntry();
		this._globalEntry = new MosaicGlobalRestrictionEntry();
	}

	get version() {
		return this._version;
	}

	get entryType() {
		return this._entryType;
	}

	get addressEntry() {
		return this._addressEntry;
	}

	get globalEntry() {
		return this._globalEntry;
	}

	set version(value) {
		this._version = value;
	}

	set entryType(value) {
		this._entryType = value;
	}

	set addressEntry(value) {
		this._addressEntry = value;
	}

	set globalEntry(value) {
		this._globalEntry = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.entryType.size;
		if (MosaicRestrictionEntryType.ADDRESS == this.entryType) {
			size += this.addressEntry.size;
		}
		if (MosaicRestrictionEntryType.GLOBAL == this.entryType) {
			size += this.globalEntry.size;
		}
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const entryType = MosaicRestrictionEntryType.deserialize(view.buffer);
		view.shift(entryType.size);
		let addressEntry = null
		if (MosaicRestrictionEntryType.ADDRESS == entryType) {
			addressEntry = MosaicAddressRestrictionEntry.deserialize(view.buffer);
			view.shift(addressEntry.size);
		}
		let globalEntry = null
		if (MosaicRestrictionEntryType.GLOBAL == entryType) {
			globalEntry = MosaicGlobalRestrictionEntry.deserialize(view.buffer);
			view.shift(globalEntry.size);
		}

		const instance = new MosaicRestrictionEntry();
		instance._version = version;
		instance._entryType = entryType;
		instance._addressEntry = addressEntry;
		instance._globalEntry = globalEntry;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._entryType.serialize());
		if (MosaicRestrictionEntryType.ADDRESS == this.entryType) {
			buffer_.write(this._addressEntry.serialize());
		}
		if (MosaicRestrictionEntryType.GLOBAL == this.entryType) {
			buffer_.write(this._globalEntry.serialize());
		}
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `entryType: ${this._entryType.toString()}`;
		result += `addressEntry: ${this._addressEntry.toString()}`;
		result += `globalEntry: ${this._globalEntry.toString()}`;
		result += ')';
		return result;
	}
}

class LockHashAlgorithm {
	static SHA3_256 = new LockHashAlgorithm(0);

	static HASH_160 = new LockHashAlgorithm(1);

	static HASH_256 = new LockHashAlgorithm(2);

	constructor(value) {
		this.value = value;
	}

	static fromValue(value) {
		const values = [
			0, 1, 2
		];
		const keys = [
			'SHA3_256', 'HASH_160', 'HASH_256'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return LockHashAlgorithm[keys[index]];
	}

	get size() {
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}
}

class SecretLockInfo {
	static TYPE_HINTS = {
		ownerAddress: 'pod:Address',
		mosaic: 'struct:Mosaic',
		endHeight: 'pod:Height',
		status: 'enum:LockStatus',
		hashAlgorithm: 'enum:LockHashAlgorithm',
		secret: 'pod:Hash256',
		recipient: 'pod:Address'
	};

	constructor() {
		this._version = 0;
		this._ownerAddress = new Address();
		this._mosaic = new Mosaic();
		this._endHeight = new Height();
		this._status = LockStatus.UNUSED;
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._secret = new Hash256();
		this._recipient = new Address();
	}

	get version() {
		return this._version;
	}

	get ownerAddress() {
		return this._ownerAddress;
	}

	get mosaic() {
		return this._mosaic;
	}

	get endHeight() {
		return this._endHeight;
	}

	get status() {
		return this._status;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	get secret() {
		return this._secret;
	}

	get recipient() {
		return this._recipient;
	}

	set version(value) {
		this._version = value;
	}

	set ownerAddress(value) {
		this._ownerAddress = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set endHeight(value) {
		this._endHeight = value;
	}

	set status(value) {
		this._status = value;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	set secret(value) {
		this._secret = value;
	}

	set recipient(value) {
		this._recipient = value;
	}

	get size() {
		let size = 0
		size += 2;
		size += this.ownerAddress.size;
		size += this.mosaic.size;
		size += this.endHeight.size;
		size += this.status.size;
		size += this.hashAlgorithm.size;
		size += this.secret.size;
		size += this.recipient.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const ownerAddress = Address.deserialize(view.buffer);
		view.shift(ownerAddress.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const endHeight = Height.deserialize(view.buffer);
		view.shift(endHeight.size);
		const status = LockStatus.deserialize(view.buffer);
		view.shift(status.size);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shift(hashAlgorithm.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shift(secret.size);
		const recipient = Address.deserialize(view.buffer);
		view.shift(recipient.size);

		const instance = new SecretLockInfo();
		instance._version = version;
		instance._ownerAddress = ownerAddress;
		instance._mosaic = mosaic;
		instance._endHeight = endHeight;
		instance._status = status;
		instance._hashAlgorithm = hashAlgorithm;
		instance._secret = secret;
		instance._recipient = recipient;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 2, false));
		buffer_.write(this._ownerAddress.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._endHeight.serialize());
		buffer_.write(this._status.serialize());
		buffer_.write(this._hashAlgorithm.serialize());
		buffer_.write(this._secret.serialize());
		buffer_.write(this._recipient.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `ownerAddress: ${this._ownerAddress.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `endHeight: ${this._endHeight.toString()}`;
		result += `status: ${this._status.toString()}`;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}`;
		result += `secret: ${this._secret.toString()}`;
		result += `recipient: ${this._recipient.toString()}`;
		result += ')';
		return result;
	}
}

class AccountKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_KEY_LINK;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		linkedPublicKey: 'pod:PublicKey',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AccountKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountKeyLinkTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new AccountKeyLinkTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._linkedPublicKey = linkedPublicKey;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedAccountKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_KEY_LINK;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		linkedPublicKey: 'pod:PublicKey',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedAccountKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountKeyLinkTransaction.TRANSACTION_TYPE;
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new EmbeddedAccountKeyLinkTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._linkedPublicKey = linkedPublicKey;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class NodeKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NODE_KEY_LINK;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		linkedPublicKey: 'pod:PublicKey',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = NodeKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NodeKeyLinkTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new NodeKeyLinkTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._linkedPublicKey = linkedPublicKey;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedNodeKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NODE_KEY_LINK;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		linkedPublicKey: 'pod:PublicKey',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedNodeKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedNodeKeyLinkTransaction.TRANSACTION_TYPE;
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new EmbeddedNodeKeyLinkTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._linkedPublicKey = linkedPublicKey;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class Cosignature {
	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature'
	};

	constructor() {
		this._version = 0n;
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
	}

	get version() {
		return this._version;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get signature() {
		return this._signature;
	}

	set version(value) {
		this._version = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set signature(value) {
		this._signature = value;
	}

	get size() {
		let size = 0
		size += 8;
		size += this.signerPublicKey.size;
		size += this.signature.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);

		const instance = new Cosignature();
		instance._version = version;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 8, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(this._signature.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `signature: ${this._signature.toString()}`;
		result += ')';
		return result;
	}
}

class DetachedCosignature {
	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		parentHash: 'pod:Hash256'
	};

	constructor() {
		this._version = 0n;
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._parentHash = new Hash256();
	}

	get version() {
		return this._version;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get signature() {
		return this._signature;
	}

	get parentHash() {
		return this._parentHash;
	}

	set version(value) {
		this._version = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set signature(value) {
		this._signature = value;
	}

	set parentHash(value) {
		this._parentHash = value;
	}

	get size() {
		let size = 0
		size += 8;
		size += this.signerPublicKey.size;
		size += this.signature.size;
		size += this.parentHash.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const parentHash = Hash256.deserialize(view.buffer);
		view.shift(parentHash.size);

		const instance = new DetachedCosignature();
		instance._version = version;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._parentHash = parentHash;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this._version, 8, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(this._signature.serialize());
		buffer_.write(this._parentHash.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `signature: ${this._signature.toString()}`;
		result += `parentHash: ${this._parentHash.toString()}`;
		result += ')';
		return result;
	}
}

class AggregateCompleteTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.AGGREGATE_COMPLETE;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		transactionsHash: 'pod:Hash256',
		transactions: 'array[EmbeddedTransaction]',
		cosignatures: 'array[Cosignature]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AggregateCompleteTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AggregateCompleteTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._transactionsHash = new Hash256();
		this._transactions = [];
		this._cosignatures = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	get transactions() {
		return this._transactions;
	}

	get cosignatures() {
		return this._cosignatures;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	set transactions(value) {
		this._transactions = value;
	}

	set cosignatures(value) {
		this._cosignatures = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.transactionsHash.size;
		size += 4;
		size += 4;
		size += this.transactions.map(e => arrayHelpers.align_up(e.size, 8)).reduce((a, b) => a + b, 0);
		size += this.cosignatures.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shift(transactionsHash.size);
		const payloadSize = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const aggregateTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (aggregateTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${aggregateTransactionHeaderReserved_1})`);
		const transactions = arrayHelpers.read_variable_size_elements(view.shrinked_buffer(payloadSize), EmbeddedTransactionFactory, 8);
		view.shift(payloadSize);
		const cosignatures = arrayHelpers.read_array(view.buffer, Cosignature);
		view.shift(cosignatures.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AggregateCompleteTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._transactionsHash = transactionsHash;
		instance._transactions = transactions;
		instance._cosignatures = cosignatures;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._transactionsHash.serialize());
		buffer_.write(converter.intToBytes(this.transactions.map(e => arrayHelpers.align_up(e.size, 8)).reduce((a, b) => a + b, 0), 4, false)); /* bound: payload_size */ 
		buffer_.write(converter.intToBytes(this._aggregateTransactionHeaderReserved_1, 4, false));
		arrayHelpers.write_variable_size_elements(buffer_, this._transactions, 8);
		arrayHelpers.write_array(buffer_, this._cosignatures);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `transactionsHash: ${this._transactionsHash.toString()}`;
		result += `transactions: ${this._transactions.map(e => e.toString())}`;
		result += `cosignatures: ${this._cosignatures.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AggregateBondedTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.AGGREGATE_BONDED;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		transactionsHash: 'pod:Hash256',
		transactions: 'array[EmbeddedTransaction]',
		cosignatures: 'array[Cosignature]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AggregateBondedTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AggregateBondedTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._transactionsHash = new Hash256();
		this._transactions = [];
		this._cosignatures = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	get transactions() {
		return this._transactions;
	}

	get cosignatures() {
		return this._cosignatures;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	set transactions(value) {
		this._transactions = value;
	}

	set cosignatures(value) {
		this._cosignatures = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.transactionsHash.size;
		size += 4;
		size += 4;
		size += this.transactions.map(e => arrayHelpers.align_up(e.size, 8)).reduce((a, b) => a + b, 0);
		size += this.cosignatures.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shift(transactionsHash.size);
		const payloadSize = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		const aggregateTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (aggregateTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${aggregateTransactionHeaderReserved_1})`);
		const transactions = arrayHelpers.read_variable_size_elements(view.shrinked_buffer(payloadSize), EmbeddedTransactionFactory, 8);
		view.shift(payloadSize);
		const cosignatures = arrayHelpers.read_array(view.buffer, Cosignature);
		view.shift(cosignatures.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AggregateBondedTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._transactionsHash = transactionsHash;
		instance._transactions = transactions;
		instance._cosignatures = cosignatures;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._transactionsHash.serialize());
		buffer_.write(converter.intToBytes(this.transactions.map(e => arrayHelpers.align_up(e.size, 8)).reduce((a, b) => a + b, 0), 4, false)); /* bound: payload_size */ 
		buffer_.write(converter.intToBytes(this._aggregateTransactionHeaderReserved_1, 4, false));
		arrayHelpers.write_variable_size_elements(buffer_, this._transactions, 8);
		arrayHelpers.write_array(buffer_, this._cosignatures);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `transactionsHash: ${this._transactionsHash.toString()}`;
		result += `transactions: ${this._transactions.map(e => e.toString())}`;
		result += `cosignatures: ${this._cosignatures.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class VotingKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.VOTING_KEY_LINK;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		linkedPublicKey: 'pod:VotingPublicKey',
		startEpoch: 'pod:FinalizationEpoch',
		endEpoch: 'pod:FinalizationEpoch',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = VotingKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = VotingKeyLinkTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new VotingPublicKey();
		this._startEpoch = new FinalizationEpoch();
		this._endEpoch = new FinalizationEpoch();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get startEpoch() {
		return this._startEpoch;
	}

	get endEpoch() {
		return this._endEpoch;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set startEpoch(value) {
		this._startEpoch = value;
	}

	set endEpoch(value) {
		this._endEpoch = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.linkedPublicKey.size;
		size += this.startEpoch.size;
		size += this.endEpoch.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const linkedPublicKey = VotingPublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const startEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(endEpoch.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new VotingKeyLinkTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._linkedPublicKey = linkedPublicKey;
		instance._startEpoch = startEpoch;
		instance._endEpoch = endEpoch;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._startEpoch.serialize());
		buffer_.write(this._endEpoch.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `startEpoch: ${this._startEpoch.toString()}`;
		result += `endEpoch: ${this._endEpoch.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedVotingKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.VOTING_KEY_LINK;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		linkedPublicKey: 'pod:VotingPublicKey',
		startEpoch: 'pod:FinalizationEpoch',
		endEpoch: 'pod:FinalizationEpoch',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedVotingKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedVotingKeyLinkTransaction.TRANSACTION_TYPE;
		this._linkedPublicKey = new VotingPublicKey();
		this._startEpoch = new FinalizationEpoch();
		this._endEpoch = new FinalizationEpoch();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get startEpoch() {
		return this._startEpoch;
	}

	get endEpoch() {
		return this._endEpoch;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set startEpoch(value) {
		this._startEpoch = value;
	}

	set endEpoch(value) {
		this._endEpoch = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.startEpoch.size;
		size += this.endEpoch.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const linkedPublicKey = VotingPublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const startEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shift(endEpoch.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new EmbeddedVotingKeyLinkTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._linkedPublicKey = linkedPublicKey;
		instance._startEpoch = startEpoch;
		instance._endEpoch = endEpoch;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._startEpoch.serialize());
		buffer_.write(this._endEpoch.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `startEpoch: ${this._startEpoch.toString()}`;
		result += `endEpoch: ${this._endEpoch.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class VrfKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.VRF_KEY_LINK;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		linkedPublicKey: 'pod:PublicKey',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = VrfKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = VrfKeyLinkTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new VrfKeyLinkTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._linkedPublicKey = linkedPublicKey;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedVrfKeyLinkTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.VRF_KEY_LINK;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		linkedPublicKey: 'pod:PublicKey',
		linkAction: 'enum:LinkAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedVrfKeyLinkTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedVrfKeyLinkTransaction.TRANSACTION_TYPE;
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	get linkAction() {
		return this._linkAction;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shift(linkAction.size);

		const instance = new EmbeddedVrfKeyLinkTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._linkedPublicKey = linkedPublicKey;
		instance._linkAction = linkAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._linkedPublicKey.serialize());
		buffer_.write(this._linkAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}`;
		result += `linkAction: ${this._linkAction.toString()}`;
		result += ')';
		return result;
	}
}

class HashLockTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.HASH_LOCK;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaic: 'struct:UnresolvedMosaic',
		duration: 'pod:BlockDuration',
		hash: 'pod:Hash256'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = HashLockTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = HashLockTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hash = new Hash256();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get mosaic() {
		return this._mosaic;
	}

	get duration() {
		return this._duration;
	}

	get hash() {
		return this._hash;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.mosaic.size;
		size += this.duration.size;
		size += this.hash.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shift(hash.size);

		const instance = new HashLockTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaic = mosaic;
		instance._duration = duration;
		instance._hash = hash;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._duration.serialize());
		buffer_.write(this._hash.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `hash: ${this._hash.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedHashLockTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.HASH_LOCK;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		mosaic: 'struct:UnresolvedMosaic',
		duration: 'pod:BlockDuration',
		hash: 'pod:Hash256'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedHashLockTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedHashLockTransaction.TRANSACTION_TYPE;
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hash = new Hash256();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get mosaic() {
		return this._mosaic;
	}

	get duration() {
		return this._duration;
	}

	get hash() {
		return this._hash;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.duration.size;
		size += this.hash.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shift(hash.size);

		const instance = new EmbeddedHashLockTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._duration = duration;
		instance._hash = hash;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._duration.serialize());
		buffer_.write(this._hash.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `hash: ${this._hash.toString()}`;
		result += ')';
		return result;
	}
}

class SecretLockTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.SECRET_LOCK;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:UnresolvedAddress',
		secret: 'pod:Hash256',
		mosaic: 'struct:UnresolvedMosaic',
		duration: 'pod:BlockDuration',
		hashAlgorithm: 'enum:LockHashAlgorithm'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = SecretLockTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = SecretLockTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	get secret() {
		return this._secret;
	}

	get mosaic() {
		return this._mosaic;
	}

	get duration() {
		return this._duration;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	set secret(value) {
		this._secret = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.recipientAddress.size;
		size += this.secret.size;
		size += this.mosaic.size;
		size += this.duration.size;
		size += this.hashAlgorithm.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shift(secret.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shift(hashAlgorithm.size);

		const instance = new SecretLockTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._secret = secret;
		instance._mosaic = mosaic;
		instance._duration = duration;
		instance._hashAlgorithm = hashAlgorithm;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._recipientAddress.serialize());
		buffer_.write(this._secret.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._duration.serialize());
		buffer_.write(this._hashAlgorithm.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += `secret: ${this._secret.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedSecretLockTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.SECRET_LOCK;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		recipientAddress: 'pod:UnresolvedAddress',
		secret: 'pod:Hash256',
		mosaic: 'struct:UnresolvedMosaic',
		duration: 'pod:BlockDuration',
		hashAlgorithm: 'enum:LockHashAlgorithm'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedSecretLockTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedSecretLockTransaction.TRANSACTION_TYPE;
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	get secret() {
		return this._secret;
	}

	get mosaic() {
		return this._mosaic;
	}

	get duration() {
		return this._duration;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	set secret(value) {
		this._secret = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.recipientAddress.size;
		size += this.secret.size;
		size += this.mosaic.size;
		size += this.duration.size;
		size += this.hashAlgorithm.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shift(secret.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shift(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shift(hashAlgorithm.size);

		const instance = new EmbeddedSecretLockTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._recipientAddress = recipientAddress;
		instance._secret = secret;
		instance._mosaic = mosaic;
		instance._duration = duration;
		instance._hashAlgorithm = hashAlgorithm;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._recipientAddress.serialize());
		buffer_.write(this._secret.serialize());
		buffer_.write(this._mosaic.serialize());
		buffer_.write(this._duration.serialize());
		buffer_.write(this._hashAlgorithm.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += `secret: ${this._secret.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}`;
		result += ')';
		return result;
	}
}

class SecretProofTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.SECRET_PROOF;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:UnresolvedAddress',
		secret: 'pod:Hash256',
		hashAlgorithm: 'enum:LockHashAlgorithm',
		proof: 'bytes_array'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = SecretProofTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = SecretProofTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._proof = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	get secret() {
		return this._secret;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	get proof() {
		return this._proof;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	set secret(value) {
		this._secret = value;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	set proof(value) {
		this._proof = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.recipientAddress.size;
		size += this.secret.size;
		size += 2;
		size += this.hashAlgorithm.size;
		size += this._proof.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shift(secret.size);
		const proofSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shift(hashAlgorithm.size);
		const proof = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, proofSize);
		view.shift(proofSize);

		const instance = new SecretProofTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._secret = secret;
		instance._hashAlgorithm = hashAlgorithm;
		instance._proof = proof;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._recipientAddress.serialize());
		buffer_.write(this._secret.serialize());
		buffer_.write(converter.intToBytes(this._proof.length, 2, false)); /* bound: proof_size */ 
		buffer_.write(this._hashAlgorithm.serialize());
		buffer_.write(this._proof);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += `secret: ${this._secret.toString()}`;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}`;
		result += `proof: ${this._proof /* FIXME */}`;
		result += ')';
		return result;
	}
}

class EmbeddedSecretProofTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.SECRET_PROOF;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		recipientAddress: 'pod:UnresolvedAddress',
		secret: 'pod:Hash256',
		hashAlgorithm: 'enum:LockHashAlgorithm',
		proof: 'bytes_array'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedSecretProofTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedSecretProofTransaction.TRANSACTION_TYPE;
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._proof = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	get secret() {
		return this._secret;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	get proof() {
		return this._proof;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	set secret(value) {
		this._secret = value;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	set proof(value) {
		this._proof = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.recipientAddress.size;
		size += this.secret.size;
		size += 2;
		size += this.hashAlgorithm.size;
		size += this._proof.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shift(secret.size);
		const proofSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shift(hashAlgorithm.size);
		const proof = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, proofSize);
		view.shift(proofSize);

		const instance = new EmbeddedSecretProofTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._recipientAddress = recipientAddress;
		instance._secret = secret;
		instance._hashAlgorithm = hashAlgorithm;
		instance._proof = proof;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._recipientAddress.serialize());
		buffer_.write(this._secret.serialize());
		buffer_.write(converter.intToBytes(this._proof.length, 2, false)); /* bound: proof_size */ 
		buffer_.write(this._hashAlgorithm.serialize());
		buffer_.write(this._proof);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += `secret: ${this._secret.toString()}`;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}`;
		result += `proof: ${this._proof /* FIXME */}`;
		result += ')';
		return result;
	}
}

class AccountMetadataTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_METADATA;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		targetAddress: 'pod:UnresolvedAddress',
		value: 'bytes_array'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AccountMetadataTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountMetadataTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	get value() {
		return this._value;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.targetAddress.size;
		size += 8;
		size += 2;
		size += 2;
		size += this._value.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shift(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shift(valueSize);

		const instance = new AccountMetadataTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._valueSizeDelta = valueSizeDelta;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer_.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer_.write(converter.intToBytes(this._value.length, 2, false)); /* bound: value_size */ 
		buffer_.write(this._value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${"0x".concat(this._scopedMetadataKey.toString(16))}`;
		result += `valueSizeDelta: ${"0x".concat(this._valueSizeDelta.toString(16))}`;
		result += `value: ${this._value /* FIXME */}`;
		result += ')';
		return result;
	}
}

class EmbeddedAccountMetadataTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_METADATA;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		targetAddress: 'pod:UnresolvedAddress',
		value: 'bytes_array'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedAccountMetadataTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountMetadataTransaction.TRANSACTION_TYPE;
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	get value() {
		return this._value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.targetAddress.size;
		size += 8;
		size += 2;
		size += 2;
		size += this._value.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shift(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shift(valueSize);

		const instance = new EmbeddedAccountMetadataTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._valueSizeDelta = valueSizeDelta;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer_.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer_.write(converter.intToBytes(this._value.length, 2, false)); /* bound: value_size */ 
		buffer_.write(this._value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${"0x".concat(this._scopedMetadataKey.toString(16))}`;
		result += `valueSizeDelta: ${"0x".concat(this._valueSizeDelta.toString(16))}`;
		result += `value: ${this._value /* FIXME */}`;
		result += ')';
		return result;
	}
}

class MosaicMetadataTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_METADATA;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		targetAddress: 'pod:UnresolvedAddress',
		targetMosaicId: 'pod:UnresolvedMosaicId',
		value: 'bytes_array'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicMetadataTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicMetadataTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._targetMosaicId = new UnresolvedMosaicId();
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get targetMosaicId() {
		return this._targetMosaicId;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	get value() {
		return this._value;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set targetMosaicId(value) {
		this._targetMosaicId = value;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.targetAddress.size;
		size += 8;
		size += this.targetMosaicId.size;
		size += 2;
		size += 2;
		size += this._value.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const targetMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(targetMosaicId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shift(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shift(valueSize);

		const instance = new MosaicMetadataTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._targetMosaicId = targetMosaicId;
		instance._valueSizeDelta = valueSizeDelta;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer_.write(this._targetMosaicId.serialize());
		buffer_.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer_.write(converter.intToBytes(this._value.length, 2, false)); /* bound: value_size */ 
		buffer_.write(this._value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${"0x".concat(this._scopedMetadataKey.toString(16))}`;
		result += `targetMosaicId: ${this._targetMosaicId.toString()}`;
		result += `valueSizeDelta: ${"0x".concat(this._valueSizeDelta.toString(16))}`;
		result += `value: ${this._value /* FIXME */}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicMetadataTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_METADATA;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		targetAddress: 'pod:UnresolvedAddress',
		targetMosaicId: 'pod:UnresolvedMosaicId',
		value: 'bytes_array'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicMetadataTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicMetadataTransaction.TRANSACTION_TYPE;
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._targetMosaicId = new UnresolvedMosaicId();
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get targetMosaicId() {
		return this._targetMosaicId;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	get value() {
		return this._value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set targetMosaicId(value) {
		this._targetMosaicId = value;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.targetAddress.size;
		size += 8;
		size += this.targetMosaicId.size;
		size += 2;
		size += 2;
		size += this._value.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const targetMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(targetMosaicId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shift(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shift(valueSize);

		const instance = new EmbeddedMosaicMetadataTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._targetMosaicId = targetMosaicId;
		instance._valueSizeDelta = valueSizeDelta;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer_.write(this._targetMosaicId.serialize());
		buffer_.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer_.write(converter.intToBytes(this._value.length, 2, false)); /* bound: value_size */ 
		buffer_.write(this._value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${"0x".concat(this._scopedMetadataKey.toString(16))}`;
		result += `targetMosaicId: ${this._targetMosaicId.toString()}`;
		result += `valueSizeDelta: ${"0x".concat(this._valueSizeDelta.toString(16))}`;
		result += `value: ${this._value /* FIXME */}`;
		result += ')';
		return result;
	}
}

class NamespaceMetadataTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NAMESPACE_METADATA;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		targetAddress: 'pod:UnresolvedAddress',
		targetNamespaceId: 'pod:NamespaceId',
		value: 'bytes_array'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = NamespaceMetadataTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NamespaceMetadataTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._targetNamespaceId = new NamespaceId();
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get targetNamespaceId() {
		return this._targetNamespaceId;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	get value() {
		return this._value;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set targetNamespaceId(value) {
		this._targetNamespaceId = value;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.targetAddress.size;
		size += 8;
		size += this.targetNamespaceId.size;
		size += 2;
		size += 2;
		size += this._value.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const targetNamespaceId = NamespaceId.deserialize(view.buffer);
		view.shift(targetNamespaceId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shift(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shift(valueSize);

		const instance = new NamespaceMetadataTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._targetNamespaceId = targetNamespaceId;
		instance._valueSizeDelta = valueSizeDelta;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer_.write(this._targetNamespaceId.serialize());
		buffer_.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer_.write(converter.intToBytes(this._value.length, 2, false)); /* bound: value_size */ 
		buffer_.write(this._value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${"0x".concat(this._scopedMetadataKey.toString(16))}`;
		result += `targetNamespaceId: ${this._targetNamespaceId.toString()}`;
		result += `valueSizeDelta: ${"0x".concat(this._valueSizeDelta.toString(16))}`;
		result += `value: ${this._value /* FIXME */}`;
		result += ')';
		return result;
	}
}

class EmbeddedNamespaceMetadataTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NAMESPACE_METADATA;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		targetAddress: 'pod:UnresolvedAddress',
		targetNamespaceId: 'pod:NamespaceId',
		value: 'bytes_array'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedNamespaceMetadataTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedNamespaceMetadataTransaction.TRANSACTION_TYPE;
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._targetNamespaceId = new NamespaceId();
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	get targetNamespaceId() {
		return this._targetNamespaceId;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	get value() {
		return this._value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	set targetNamespaceId(value) {
		this._targetNamespaceId = value;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	set value(value) {
		this._value = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.targetAddress.size;
		size += 8;
		size += this.targetNamespaceId.size;
		size += 2;
		size += 2;
		size += this._value.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const targetNamespaceId = NamespaceId.deserialize(view.buffer);
		view.shift(targetNamespaceId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shift(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shift(valueSize);

		const instance = new EmbeddedNamespaceMetadataTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._targetAddress = targetAddress;
		instance._scopedMetadataKey = scopedMetadataKey;
		instance._targetNamespaceId = targetNamespaceId;
		instance._valueSizeDelta = valueSizeDelta;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._targetAddress.serialize());
		buffer_.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer_.write(this._targetNamespaceId.serialize());
		buffer_.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer_.write(converter.intToBytes(this._value.length, 2, false)); /* bound: value_size */ 
		buffer_.write(this._value);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += `scopedMetadataKey: ${"0x".concat(this._scopedMetadataKey.toString(16))}`;
		result += `targetNamespaceId: ${this._targetNamespaceId.toString()}`;
		result += `valueSizeDelta: ${"0x".concat(this._valueSizeDelta.toString(16))}`;
		result += `value: ${this._value /* FIXME */}`;
		result += ')';
		return result;
	}
}

class MosaicDefinitionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_DEFINITION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		id: 'pod:MosaicId',
		duration: 'pod:BlockDuration',
		nonce: 'pod:MosaicNonce',
		flags: 'enum:MosaicFlags'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicDefinitionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicDefinitionTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._id = new MosaicId();
		this._duration = new BlockDuration();
		this._nonce = new MosaicNonce();
		this._flags = MosaicFlags.NONE;
		this._divisibility = 0;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get id() {
		return this._id;
	}

	get duration() {
		return this._duration;
	}

	get nonce() {
		return this._nonce;
	}

	get flags() {
		return this._flags;
	}

	get divisibility() {
		return this._divisibility;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set id(value) {
		this._id = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set nonce(value) {
		this._nonce = value;
	}

	set flags(value) {
		this._flags = value;
	}

	set divisibility(value) {
		this._divisibility = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.id.size;
		size += this.duration.size;
		size += this.nonce.size;
		size += this.flags.size;
		size += 1;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const id = MosaicId.deserialize(view.buffer);
		view.shift(id.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);
		const nonce = MosaicNonce.deserialize(view.buffer);
		view.shift(nonce.size);
		const flags = MosaicFlags.deserialize(view.buffer);
		view.shift(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);

		const instance = new MosaicDefinitionTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._id = id;
		instance._duration = duration;
		instance._nonce = nonce;
		instance._flags = flags;
		instance._divisibility = divisibility;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._id.serialize());
		buffer_.write(this._duration.serialize());
		buffer_.write(this._nonce.serialize());
		buffer_.write(this._flags.serialize());
		buffer_.write(converter.intToBytes(this._divisibility, 1, false));
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `id: ${this._id.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `nonce: ${this._nonce.toString()}`;
		result += `flags: ${this._flags.toString()}`;
		result += `divisibility: ${"0x".concat(this._divisibility.toString(16))}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicDefinitionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_DEFINITION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		id: 'pod:MosaicId',
		duration: 'pod:BlockDuration',
		nonce: 'pod:MosaicNonce',
		flags: 'enum:MosaicFlags'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicDefinitionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicDefinitionTransaction.TRANSACTION_TYPE;
		this._id = new MosaicId();
		this._duration = new BlockDuration();
		this._nonce = new MosaicNonce();
		this._flags = MosaicFlags.NONE;
		this._divisibility = 0;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get id() {
		return this._id;
	}

	get duration() {
		return this._duration;
	}

	get nonce() {
		return this._nonce;
	}

	get flags() {
		return this._flags;
	}

	get divisibility() {
		return this._divisibility;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set id(value) {
		this._id = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set nonce(value) {
		this._nonce = value;
	}

	set flags(value) {
		this._flags = value;
	}

	set divisibility(value) {
		this._divisibility = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.id.size;
		size += this.duration.size;
		size += this.nonce.size;
		size += this.flags.size;
		size += 1;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const id = MosaicId.deserialize(view.buffer);
		view.shift(id.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shift(duration.size);
		const nonce = MosaicNonce.deserialize(view.buffer);
		view.shift(nonce.size);
		const flags = MosaicFlags.deserialize(view.buffer);
		view.shift(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);

		const instance = new EmbeddedMosaicDefinitionTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._id = id;
		instance._duration = duration;
		instance._nonce = nonce;
		instance._flags = flags;
		instance._divisibility = divisibility;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._id.serialize());
		buffer_.write(this._duration.serialize());
		buffer_.write(this._nonce.serialize());
		buffer_.write(this._flags.serialize());
		buffer_.write(converter.intToBytes(this._divisibility, 1, false));
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `id: ${this._id.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `nonce: ${this._nonce.toString()}`;
		result += `flags: ${this._flags.toString()}`;
		result += `divisibility: ${"0x".concat(this._divisibility.toString(16))}`;
		result += ')';
		return result;
	}
}

class MosaicSupplyChangeTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_SUPPLY_CHANGE;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicId: 'pod:UnresolvedMosaicId',
		delta: 'pod:Amount',
		action: 'enum:MosaicSupplyChangeAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicSupplyChangeTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicSupplyChangeTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicId = new UnresolvedMosaicId();
		this._delta = new Amount();
		this._action = MosaicSupplyChangeAction.DECREASE;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get delta() {
		return this._delta;
	}

	get action() {
		return this._action;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set delta(value) {
		this._delta = value;
	}

	set action(value) {
		this._action = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.mosaicId.size;
		size += this.delta.size;
		size += this.action.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const delta = Amount.deserialize(view.buffer);
		view.shift(delta.size);
		const action = MosaicSupplyChangeAction.deserialize(view.buffer);
		view.shift(action.size);

		const instance = new MosaicSupplyChangeTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicId = mosaicId;
		instance._delta = delta;
		instance._action = action;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._delta.serialize());
		buffer_.write(this._action.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `delta: ${this._delta.toString()}`;
		result += `action: ${this._action.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicSupplyChangeTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_SUPPLY_CHANGE;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		mosaicId: 'pod:UnresolvedMosaicId',
		delta: 'pod:Amount',
		action: 'enum:MosaicSupplyChangeAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicSupplyChangeTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicSupplyChangeTransaction.TRANSACTION_TYPE;
		this._mosaicId = new UnresolvedMosaicId();
		this._delta = new Amount();
		this._action = MosaicSupplyChangeAction.DECREASE;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get delta() {
		return this._delta;
	}

	get action() {
		return this._action;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set delta(value) {
		this._delta = value;
	}

	set action(value) {
		this._action = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.mosaicId.size;
		size += this.delta.size;
		size += this.action.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const delta = Amount.deserialize(view.buffer);
		view.shift(delta.size);
		const action = MosaicSupplyChangeAction.deserialize(view.buffer);
		view.shift(action.size);

		const instance = new EmbeddedMosaicSupplyChangeTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._mosaicId = mosaicId;
		instance._delta = delta;
		instance._action = action;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._delta.serialize());
		buffer_.write(this._action.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `delta: ${this._delta.toString()}`;
		result += `action: ${this._action.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicSupplyRevocationTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_SUPPLY_REVOCATION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		sourceAddress: 'pod:UnresolvedAddress',
		mosaic: 'struct:UnresolvedMosaic'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicSupplyRevocationTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicSupplyRevocationTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._sourceAddress = new UnresolvedAddress();
		this._mosaic = new UnresolvedMosaic();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get sourceAddress() {
		return this._sourceAddress;
	}

	get mosaic() {
		return this._mosaic;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set sourceAddress(value) {
		this._sourceAddress = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.sourceAddress.size;
		size += this.mosaic.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const sourceAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(sourceAddress.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shift(mosaic.size);

		const instance = new MosaicSupplyRevocationTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._sourceAddress = sourceAddress;
		instance._mosaic = mosaic;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._sourceAddress.serialize());
		buffer_.write(this._mosaic.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `sourceAddress: ${this._sourceAddress.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicSupplyRevocationTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_SUPPLY_REVOCATION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		sourceAddress: 'pod:UnresolvedAddress',
		mosaic: 'struct:UnresolvedMosaic'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicSupplyRevocationTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicSupplyRevocationTransaction.TRANSACTION_TYPE;
		this._sourceAddress = new UnresolvedAddress();
		this._mosaic = new UnresolvedMosaic();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get sourceAddress() {
		return this._sourceAddress;
	}

	get mosaic() {
		return this._mosaic;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set sourceAddress(value) {
		this._sourceAddress = value;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.sourceAddress.size;
		size += this.mosaic.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const sourceAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(sourceAddress.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shift(mosaic.size);

		const instance = new EmbeddedMosaicSupplyRevocationTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._sourceAddress = sourceAddress;
		instance._mosaic = mosaic;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._sourceAddress.serialize());
		buffer_.write(this._mosaic.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `sourceAddress: ${this._sourceAddress.toString()}`;
		result += `mosaic: ${this._mosaic.toString()}`;
		result += ')';
		return result;
	}
}

class MultisigAccountModificationTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_ACCOUNT_MODIFICATION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		addressAdditions: 'array[UnresolvedAddress]',
		addressDeletions: 'array[UnresolvedAddress]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MultisigAccountModificationTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MultisigAccountModificationTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._minRemovalDelta = 0;
		this._minApprovalDelta = 0;
		this._addressAdditions = [];
		this._addressDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._multisigAccountModificationTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get minRemovalDelta() {
		return this._minRemovalDelta;
	}

	get minApprovalDelta() {
		return this._minApprovalDelta;
	}

	get addressAdditions() {
		return this._addressAdditions;
	}

	get addressDeletions() {
		return this._addressDeletions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set minRemovalDelta(value) {
		this._minRemovalDelta = value;
	}

	set minApprovalDelta(value) {
		this._minApprovalDelta = value;
	}

	set addressAdditions(value) {
		this._addressAdditions = value;
	}

	set addressDeletions(value) {
		this._addressDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 1;
		size += 1;
		size += 1;
		size += 1;
		size += 4;
		size += this.addressAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.addressDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const minRemovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shift(1);
		const minApprovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shift(1);
		const addressAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const addressDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const multisigAccountModificationTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (multisigAccountModificationTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${multisigAccountModificationTransactionBodyReserved_1})`);
		const addressAdditions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, addressAdditionsCount);
		view.shift(addressAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const addressDeletions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, addressDeletionsCount);
		view.shift(addressDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new MultisigAccountModificationTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._minRemovalDelta = minRemovalDelta;
		instance._minApprovalDelta = minApprovalDelta;
		instance._addressAdditions = addressAdditions;
		instance._addressDeletions = addressDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(converter.intToBytes(this._minRemovalDelta, 1, true));
		buffer_.write(converter.intToBytes(this._minApprovalDelta, 1, true));
		buffer_.write(converter.intToBytes(this._addressAdditions.length, 1, false)); /* bound: address_additions_count */ 
		buffer_.write(converter.intToBytes(this._addressDeletions.length, 1, false)); /* bound: address_deletions_count */ 
		buffer_.write(converter.intToBytes(this._multisigAccountModificationTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._addressAdditions);
		arrayHelpers.write_array(buffer_, this._addressDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `minRemovalDelta: ${"0x".concat(this._minRemovalDelta.toString(16))}`;
		result += `minApprovalDelta: ${"0x".concat(this._minApprovalDelta.toString(16))}`;
		result += `addressAdditions: ${this._addressAdditions.map(e => e.toString())}`;
		result += `addressDeletions: ${this._addressDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class EmbeddedMultisigAccountModificationTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_ACCOUNT_MODIFICATION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		addressAdditions: 'array[UnresolvedAddress]',
		addressDeletions: 'array[UnresolvedAddress]'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMultisigAccountModificationTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMultisigAccountModificationTransaction.TRANSACTION_TYPE;
		this._minRemovalDelta = 0;
		this._minApprovalDelta = 0;
		this._addressAdditions = [];
		this._addressDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._multisigAccountModificationTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get minRemovalDelta() {
		return this._minRemovalDelta;
	}

	get minApprovalDelta() {
		return this._minApprovalDelta;
	}

	get addressAdditions() {
		return this._addressAdditions;
	}

	get addressDeletions() {
		return this._addressDeletions;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set minRemovalDelta(value) {
		this._minRemovalDelta = value;
	}

	set minApprovalDelta(value) {
		this._minApprovalDelta = value;
	}

	set addressAdditions(value) {
		this._addressAdditions = value;
	}

	set addressDeletions(value) {
		this._addressDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += 1;
		size += 1;
		size += 1;
		size += 1;
		size += 4;
		size += this.addressAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.addressDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const minRemovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shift(1);
		const minApprovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shift(1);
		const addressAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const addressDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const multisigAccountModificationTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (multisigAccountModificationTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${multisigAccountModificationTransactionBodyReserved_1})`);
		const addressAdditions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, addressAdditionsCount);
		view.shift(addressAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const addressDeletions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, addressDeletionsCount);
		view.shift(addressDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new EmbeddedMultisigAccountModificationTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._minRemovalDelta = minRemovalDelta;
		instance._minApprovalDelta = minApprovalDelta;
		instance._addressAdditions = addressAdditions;
		instance._addressDeletions = addressDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(converter.intToBytes(this._minRemovalDelta, 1, true));
		buffer_.write(converter.intToBytes(this._minApprovalDelta, 1, true));
		buffer_.write(converter.intToBytes(this._addressAdditions.length, 1, false)); /* bound: address_additions_count */ 
		buffer_.write(converter.intToBytes(this._addressDeletions.length, 1, false)); /* bound: address_deletions_count */ 
		buffer_.write(converter.intToBytes(this._multisigAccountModificationTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._addressAdditions);
		arrayHelpers.write_array(buffer_, this._addressDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `minRemovalDelta: ${"0x".concat(this._minRemovalDelta.toString(16))}`;
		result += `minApprovalDelta: ${"0x".concat(this._minApprovalDelta.toString(16))}`;
		result += `addressAdditions: ${this._addressAdditions.map(e => e.toString())}`;
		result += `addressDeletions: ${this._addressDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AddressAliasTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ADDRESS_ALIAS;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		namespaceId: 'pod:NamespaceId',
		address: 'pod:Address',
		aliasAction: 'enum:AliasAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AddressAliasTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AddressAliasTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._namespaceId = new NamespaceId();
		this._address = new Address();
		this._aliasAction = AliasAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	get address() {
		return this._address;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	set address(value) {
		this._address = value;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.namespaceId.size;
		size += this.address.size;
		size += this.aliasAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shift(namespaceId.size);
		const address = Address.deserialize(view.buffer);
		view.shift(address.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shift(aliasAction.size);

		const instance = new AddressAliasTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._namespaceId = namespaceId;
		instance._address = address;
		instance._aliasAction = aliasAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._namespaceId.serialize());
		buffer_.write(this._address.serialize());
		buffer_.write(this._aliasAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `namespaceId: ${this._namespaceId.toString()}`;
		result += `address: ${this._address.toString()}`;
		result += `aliasAction: ${this._aliasAction.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedAddressAliasTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ADDRESS_ALIAS;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		namespaceId: 'pod:NamespaceId',
		address: 'pod:Address',
		aliasAction: 'enum:AliasAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedAddressAliasTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAddressAliasTransaction.TRANSACTION_TYPE;
		this._namespaceId = new NamespaceId();
		this._address = new Address();
		this._aliasAction = AliasAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	get address() {
		return this._address;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	set address(value) {
		this._address = value;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.namespaceId.size;
		size += this.address.size;
		size += this.aliasAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shift(namespaceId.size);
		const address = Address.deserialize(view.buffer);
		view.shift(address.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shift(aliasAction.size);

		const instance = new EmbeddedAddressAliasTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._namespaceId = namespaceId;
		instance._address = address;
		instance._aliasAction = aliasAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._namespaceId.serialize());
		buffer_.write(this._address.serialize());
		buffer_.write(this._aliasAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `namespaceId: ${this._namespaceId.toString()}`;
		result += `address: ${this._address.toString()}`;
		result += `aliasAction: ${this._aliasAction.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicAliasTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_ALIAS;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		namespaceId: 'pod:NamespaceId',
		mosaicId: 'pod:MosaicId',
		aliasAction: 'enum:AliasAction'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicAliasTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicAliasTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._namespaceId = new NamespaceId();
		this._mosaicId = new MosaicId();
		this._aliasAction = AliasAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.namespaceId.size;
		size += this.mosaicId.size;
		size += this.aliasAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shift(namespaceId.size);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shift(aliasAction.size);

		const instance = new MosaicAliasTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._namespaceId = namespaceId;
		instance._mosaicId = mosaicId;
		instance._aliasAction = aliasAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._namespaceId.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._aliasAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `namespaceId: ${this._namespaceId.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `aliasAction: ${this._aliasAction.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicAliasTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_ALIAS;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		namespaceId: 'pod:NamespaceId',
		mosaicId: 'pod:MosaicId',
		aliasAction: 'enum:AliasAction'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicAliasTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicAliasTransaction.TRANSACTION_TYPE;
		this._namespaceId = new NamespaceId();
		this._mosaicId = new MosaicId();
		this._aliasAction = AliasAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.namespaceId.size;
		size += this.mosaicId.size;
		size += this.aliasAction.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shift(namespaceId.size);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shift(aliasAction.size);

		const instance = new EmbeddedMosaicAliasTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._namespaceId = namespaceId;
		instance._mosaicId = mosaicId;
		instance._aliasAction = aliasAction;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._namespaceId.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._aliasAction.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `namespaceId: ${this._namespaceId.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `aliasAction: ${this._aliasAction.toString()}`;
		result += ')';
		return result;
	}
}

class NamespaceRegistrationTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NAMESPACE_REGISTRATION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		duration: 'pod:BlockDuration',
		parentId: 'pod:NamespaceId',
		id: 'pod:NamespaceId',
		registrationType: 'enum:NamespaceRegistrationType',
		name: 'bytes_array'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = NamespaceRegistrationTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NamespaceRegistrationTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._duration = new BlockDuration();
		this._parentId = new NamespaceId();
		this._id = new NamespaceId();
		this._registrationType = NamespaceRegistrationType.ROOT;
		this._name = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get duration() {
		return this._duration;
	}

	get parentId() {
		return this._parentId;
	}

	get id() {
		return this._id;
	}

	get registrationType() {
		return this._registrationType;
	}

	get name() {
		return this._name;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set parentId(value) {
		this._parentId = value;
	}

	set id(value) {
		this._id = value;
	}

	set registrationType(value) {
		this._registrationType = value;
	}

	set name(value) {
		this._name = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		if (NamespaceRegistrationType.ROOT == this.registrationType) {
			size += this.duration.size;
		}
		if (NamespaceRegistrationType.CHILD == this.registrationType) {
			size += this.parentId.size;
		}
		size += this.id.size;
		size += this.registrationType.size;
		size += 1;
		size += this._name.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		// deserialize to temporary buffer for further processing
		const duration_temporary = BlockDuration.deserialize(view.buffer)
		const registration_type_condition = view.shrinked_buffer(duration_temporary.size)
		view.shift(duration_temporary.size);  // skip temporary

		const id = NamespaceId.deserialize(view.buffer);
		view.shift(id.size);
		const registrationType = NamespaceRegistrationType.deserialize(view.buffer);
		view.shift(registrationType.size);
		let duration = null
		if (NamespaceRegistrationType.ROOT == registrationType) {
			duration = BlockDuration.deserialize(registration_type_condition);
		}
		let parentId = null
		if (NamespaceRegistrationType.CHILD == registrationType) {
			parentId = NamespaceId.deserialize(registration_type_condition);
		}
		const nameSize = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shift(nameSize);

		const instance = new NamespaceRegistrationTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._duration = duration;
		instance._parentId = parentId;
		instance._id = id;
		instance._registrationType = registrationType;
		instance._name = name;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		if (NamespaceRegistrationType.ROOT == this.registrationType) {
			buffer_.write(this._duration.serialize());
		}
		if (NamespaceRegistrationType.CHILD == this.registrationType) {
			buffer_.write(this._parentId.serialize());
		}
		buffer_.write(this._id.serialize());
		buffer_.write(this._registrationType.serialize());
		buffer_.write(converter.intToBytes(this._name.length, 1, false)); /* bound: name_size */ 
		buffer_.write(this._name);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `parentId: ${this._parentId.toString()}`;
		result += `id: ${this._id.toString()}`;
		result += `registrationType: ${this._registrationType.toString()}`;
		result += `name: ${this._name /* FIXME */}`;
		result += ')';
		return result;
	}
}

class EmbeddedNamespaceRegistrationTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NAMESPACE_REGISTRATION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		duration: 'pod:BlockDuration',
		parentId: 'pod:NamespaceId',
		id: 'pod:NamespaceId',
		registrationType: 'enum:NamespaceRegistrationType',
		name: 'bytes_array'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedNamespaceRegistrationTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedNamespaceRegistrationTransaction.TRANSACTION_TYPE;
		this._duration = new BlockDuration();
		this._parentId = new NamespaceId();
		this._id = new NamespaceId();
		this._registrationType = NamespaceRegistrationType.ROOT;
		this._name = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get duration() {
		return this._duration;
	}

	get parentId() {
		return this._parentId;
	}

	get id() {
		return this._id;
	}

	get registrationType() {
		return this._registrationType;
	}

	get name() {
		return this._name;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set duration(value) {
		this._duration = value;
	}

	set parentId(value) {
		this._parentId = value;
	}

	set id(value) {
		this._id = value;
	}

	set registrationType(value) {
		this._registrationType = value;
	}

	set name(value) {
		this._name = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		if (NamespaceRegistrationType.ROOT == this.registrationType) {
			size += this.duration.size;
		}
		if (NamespaceRegistrationType.CHILD == this.registrationType) {
			size += this.parentId.size;
		}
		size += this.id.size;
		size += this.registrationType.size;
		size += 1;
		size += this._name.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		// deserialize to temporary buffer for further processing
		const duration_temporary = BlockDuration.deserialize(view.buffer)
		const registration_type_condition = view.shrinked_buffer(duration_temporary.size)
		view.shift(duration_temporary.size);  // skip temporary

		const id = NamespaceId.deserialize(view.buffer);
		view.shift(id.size);
		const registrationType = NamespaceRegistrationType.deserialize(view.buffer);
		view.shift(registrationType.size);
		let duration = null
		if (NamespaceRegistrationType.ROOT == registrationType) {
			duration = BlockDuration.deserialize(registration_type_condition);
		}
		let parentId = null
		if (NamespaceRegistrationType.CHILD == registrationType) {
			parentId = NamespaceId.deserialize(registration_type_condition);
		}
		const nameSize = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shift(nameSize);

		const instance = new EmbeddedNamespaceRegistrationTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._duration = duration;
		instance._parentId = parentId;
		instance._id = id;
		instance._registrationType = registrationType;
		instance._name = name;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		if (NamespaceRegistrationType.ROOT == this.registrationType) {
			buffer_.write(this._duration.serialize());
		}
		if (NamespaceRegistrationType.CHILD == this.registrationType) {
			buffer_.write(this._parentId.serialize());
		}
		buffer_.write(this._id.serialize());
		buffer_.write(this._registrationType.serialize());
		buffer_.write(converter.intToBytes(this._name.length, 1, false)); /* bound: name_size */ 
		buffer_.write(this._name);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `duration: ${this._duration.toString()}`;
		result += `parentId: ${this._parentId.toString()}`;
		result += `id: ${this._id.toString()}`;
		result += `registrationType: ${this._registrationType.toString()}`;
		result += `name: ${this._name /* FIXME */}`;
		result += ')';
		return result;
	}
}

class AccountAddressRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_ADDRESS_RESTRICTION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		restrictionFlags: 'enum:AccountRestrictionFlags',
		restrictionAdditions: 'array[UnresolvedAddress]',
		restrictionDeletions: 'array[UnresolvedAddress]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AccountAddressRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountAddressRestrictionTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.restrictionFlags.size;
		size += 1;
		size += 1;
		size += 4;
		size += this.restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (accountRestrictionTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, restrictionAdditionsCount);
		view.shift(restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const restrictionDeletions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, restrictionDeletionsCount);
		view.shift(restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountAddressRestrictionTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._restrictionFlags = restrictionFlags;
		instance._restrictionAdditions = restrictionAdditions;
		instance._restrictionDeletions = restrictionDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._restrictionFlags.serialize());
		buffer_.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); /* bound: restriction_additions_count */ 
		buffer_.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); /* bound: restriction_deletions_count */ 
		buffer_.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._restrictionAdditions);
		arrayHelpers.write_array(buffer_, this._restrictionDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `restrictionAdditions: ${this._restrictionAdditions.map(e => e.toString())}`;
		result += `restrictionDeletions: ${this._restrictionDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class EmbeddedAccountAddressRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_ADDRESS_RESTRICTION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		restrictionFlags: 'enum:AccountRestrictionFlags',
		restrictionAdditions: 'array[UnresolvedAddress]',
		restrictionDeletions: 'array[UnresolvedAddress]'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedAccountAddressRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountAddressRestrictionTransaction.TRANSACTION_TYPE;
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.restrictionFlags.size;
		size += 1;
		size += 1;
		size += 4;
		size += this.restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (accountRestrictionTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, restrictionAdditionsCount);
		view.shift(restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const restrictionDeletions = arrayHelpers.read_array_count(view.buffer, UnresolvedAddress, restrictionDeletionsCount);
		view.shift(restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new EmbeddedAccountAddressRestrictionTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._restrictionFlags = restrictionFlags;
		instance._restrictionAdditions = restrictionAdditions;
		instance._restrictionDeletions = restrictionDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._restrictionFlags.serialize());
		buffer_.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); /* bound: restriction_additions_count */ 
		buffer_.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); /* bound: restriction_deletions_count */ 
		buffer_.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._restrictionAdditions);
		arrayHelpers.write_array(buffer_, this._restrictionDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `restrictionAdditions: ${this._restrictionAdditions.map(e => e.toString())}`;
		result += `restrictionDeletions: ${this._restrictionDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountMosaicRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_MOSAIC_RESTRICTION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		restrictionFlags: 'enum:AccountRestrictionFlags',
		restrictionAdditions: 'array[UnresolvedMosaicId]',
		restrictionDeletions: 'array[UnresolvedMosaicId]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AccountMosaicRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountMosaicRestrictionTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.restrictionFlags.size;
		size += 1;
		size += 1;
		size += 4;
		size += this.restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (accountRestrictionTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.read_array_count(view.buffer, UnresolvedMosaicId, restrictionAdditionsCount);
		view.shift(restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const restrictionDeletions = arrayHelpers.read_array_count(view.buffer, UnresolvedMosaicId, restrictionDeletionsCount);
		view.shift(restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountMosaicRestrictionTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._restrictionFlags = restrictionFlags;
		instance._restrictionAdditions = restrictionAdditions;
		instance._restrictionDeletions = restrictionDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._restrictionFlags.serialize());
		buffer_.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); /* bound: restriction_additions_count */ 
		buffer_.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); /* bound: restriction_deletions_count */ 
		buffer_.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._restrictionAdditions);
		arrayHelpers.write_array(buffer_, this._restrictionDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `restrictionAdditions: ${this._restrictionAdditions.map(e => e.toString())}`;
		result += `restrictionDeletions: ${this._restrictionDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class EmbeddedAccountMosaicRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_MOSAIC_RESTRICTION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		restrictionFlags: 'enum:AccountRestrictionFlags',
		restrictionAdditions: 'array[UnresolvedMosaicId]',
		restrictionDeletions: 'array[UnresolvedMosaicId]'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedAccountMosaicRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountMosaicRestrictionTransaction.TRANSACTION_TYPE;
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.restrictionFlags.size;
		size += 1;
		size += 1;
		size += 4;
		size += this.restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (accountRestrictionTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.read_array_count(view.buffer, UnresolvedMosaicId, restrictionAdditionsCount);
		view.shift(restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const restrictionDeletions = arrayHelpers.read_array_count(view.buffer, UnresolvedMosaicId, restrictionDeletionsCount);
		view.shift(restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new EmbeddedAccountMosaicRestrictionTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._restrictionFlags = restrictionFlags;
		instance._restrictionAdditions = restrictionAdditions;
		instance._restrictionDeletions = restrictionDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._restrictionFlags.serialize());
		buffer_.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); /* bound: restriction_additions_count */ 
		buffer_.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); /* bound: restriction_deletions_count */ 
		buffer_.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._restrictionAdditions);
		arrayHelpers.write_array(buffer_, this._restrictionDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `restrictionAdditions: ${this._restrictionAdditions.map(e => e.toString())}`;
		result += `restrictionDeletions: ${this._restrictionDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class AccountOperationRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_OPERATION_RESTRICTION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		restrictionFlags: 'enum:AccountRestrictionFlags',
		restrictionAdditions: 'array[TransactionType]',
		restrictionDeletions: 'array[TransactionType]'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = AccountOperationRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountOperationRestrictionTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.restrictionFlags.size;
		size += 1;
		size += 1;
		size += 4;
		size += this.restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (accountRestrictionTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.read_array_count(view.buffer, TransactionType, restrictionAdditionsCount);
		view.shift(restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const restrictionDeletions = arrayHelpers.read_array_count(view.buffer, TransactionType, restrictionDeletionsCount);
		view.shift(restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new AccountOperationRestrictionTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._restrictionFlags = restrictionFlags;
		instance._restrictionAdditions = restrictionAdditions;
		instance._restrictionDeletions = restrictionDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._restrictionFlags.serialize());
		buffer_.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); /* bound: restriction_additions_count */ 
		buffer_.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); /* bound: restriction_deletions_count */ 
		buffer_.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._restrictionAdditions);
		arrayHelpers.write_array(buffer_, this._restrictionDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `restrictionAdditions: ${this._restrictionAdditions.map(e => e.toString())}`;
		result += `restrictionDeletions: ${this._restrictionDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class EmbeddedAccountOperationRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_OPERATION_RESTRICTION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		restrictionFlags: 'enum:AccountRestrictionFlags',
		restrictionAdditions: 'array[TransactionType]',
		restrictionDeletions: 'array[TransactionType]'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedAccountOperationRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountOperationRestrictionTransaction.TRANSACTION_TYPE;
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.restrictionFlags.size;
		size += 1;
		size += 1;
		size += 4;
		size += this.restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this.restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0);
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shift(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (accountRestrictionTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.read_array_count(view.buffer, TransactionType, restrictionAdditionsCount);
		view.shift(restrictionAdditions.map(e => e.size).reduce((a, b) => a + b, 0));
		const restrictionDeletions = arrayHelpers.read_array_count(view.buffer, TransactionType, restrictionDeletionsCount);
		view.shift(restrictionDeletions.map(e => e.size).reduce((a, b) => a + b, 0));

		const instance = new EmbeddedAccountOperationRestrictionTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._restrictionFlags = restrictionFlags;
		instance._restrictionAdditions = restrictionAdditions;
		instance._restrictionDeletions = restrictionDeletions;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._restrictionFlags.serialize());
		buffer_.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); /* bound: restriction_additions_count */ 
		buffer_.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); /* bound: restriction_deletions_count */ 
		buffer_.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.write_array(buffer_, this._restrictionAdditions);
		arrayHelpers.write_array(buffer_, this._restrictionDeletions);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}`;
		result += `restrictionAdditions: ${this._restrictionAdditions.map(e => e.toString())}`;
		result += `restrictionDeletions: ${this._restrictionDeletions.map(e => e.toString())}`;
		result += ')';
		return result;
	}
}

class MosaicAddressRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_ADDRESS_RESTRICTION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicId: 'pod:UnresolvedMosaicId',
		targetAddress: 'pod:UnresolvedAddress'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicAddressRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicAddressRestrictionTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicId = new UnresolvedMosaicId();
		this._restrictionKey = 0n;
		this._previousRestrictionValue = 0n;
		this._newRestrictionValue = 0n;
		this._targetAddress = new UnresolvedAddress();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.mosaicId.size;
		size += 8;
		size += 8;
		size += 8;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new MosaicAddressRestrictionTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicId = mosaicId;
		instance._restrictionKey = restrictionKey;
		instance._previousRestrictionValue = previousRestrictionValue;
		instance._newRestrictionValue = newRestrictionValue;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer_.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer_.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `restrictionKey: ${"0x".concat(this._restrictionKey.toString(16))}`;
		result += `previousRestrictionValue: ${"0x".concat(this._previousRestrictionValue.toString(16))}`;
		result += `newRestrictionValue: ${"0x".concat(this._newRestrictionValue.toString(16))}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicAddressRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_ADDRESS_RESTRICTION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		mosaicId: 'pod:UnresolvedMosaicId',
		targetAddress: 'pod:UnresolvedAddress'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicAddressRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicAddressRestrictionTransaction.TRANSACTION_TYPE;
		this._mosaicId = new UnresolvedMosaicId();
		this._restrictionKey = 0n;
		this._previousRestrictionValue = 0n;
		this._newRestrictionValue = 0n;
		this._targetAddress = new UnresolvedAddress();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.mosaicId.size;
		size += 8;
		size += 8;
		size += 8;
		size += this.targetAddress.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(targetAddress.size);

		const instance = new EmbeddedMosaicAddressRestrictionTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._mosaicId = mosaicId;
		instance._restrictionKey = restrictionKey;
		instance._previousRestrictionValue = previousRestrictionValue;
		instance._newRestrictionValue = newRestrictionValue;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer_.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer_.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer_.write(this._targetAddress.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `restrictionKey: ${"0x".concat(this._restrictionKey.toString(16))}`;
		result += `previousRestrictionValue: ${"0x".concat(this._previousRestrictionValue.toString(16))}`;
		result += `newRestrictionValue: ${"0x".concat(this._newRestrictionValue.toString(16))}`;
		result += `targetAddress: ${this._targetAddress.toString()}`;
		result += ')';
		return result;
	}
}

class MosaicGlobalRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_GLOBAL_RESTRICTION;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicId: 'pod:UnresolvedMosaicId',
		referenceMosaicId: 'pod:UnresolvedMosaicId',
		previousRestrictionType: 'enum:MosaicRestrictionType',
		newRestrictionType: 'enum:MosaicRestrictionType'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = MosaicGlobalRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicGlobalRestrictionTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicId = new UnresolvedMosaicId();
		this._referenceMosaicId = new UnresolvedMosaicId();
		this._restrictionKey = 0n;
		this._previousRestrictionValue = 0n;
		this._newRestrictionValue = 0n;
		this._previousRestrictionType = MosaicRestrictionType.NONE;
		this._newRestrictionType = MosaicRestrictionType.NONE;
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get referenceMosaicId() {
		return this._referenceMosaicId;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	get previousRestrictionType() {
		return this._previousRestrictionType;
	}

	get newRestrictionType() {
		return this._newRestrictionType;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set referenceMosaicId(value) {
		this._referenceMosaicId = value;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	set previousRestrictionType(value) {
		this._previousRestrictionType = value;
	}

	set newRestrictionType(value) {
		this._newRestrictionType = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.mosaicId.size;
		size += this.referenceMosaicId.size;
		size += 8;
		size += 8;
		size += 8;
		size += this.previousRestrictionType.size;
		size += this.newRestrictionType.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const referenceMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(referenceMosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const previousRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shift(previousRestrictionType.size);
		const newRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shift(newRestrictionType.size);

		const instance = new MosaicGlobalRestrictionTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicId = mosaicId;
		instance._referenceMosaicId = referenceMosaicId;
		instance._restrictionKey = restrictionKey;
		instance._previousRestrictionValue = previousRestrictionValue;
		instance._newRestrictionValue = newRestrictionValue;
		instance._previousRestrictionType = previousRestrictionType;
		instance._newRestrictionType = newRestrictionType;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._referenceMosaicId.serialize());
		buffer_.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer_.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer_.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer_.write(this._previousRestrictionType.serialize());
		buffer_.write(this._newRestrictionType.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `referenceMosaicId: ${this._referenceMosaicId.toString()}`;
		result += `restrictionKey: ${"0x".concat(this._restrictionKey.toString(16))}`;
		result += `previousRestrictionValue: ${"0x".concat(this._previousRestrictionValue.toString(16))}`;
		result += `newRestrictionValue: ${"0x".concat(this._newRestrictionValue.toString(16))}`;
		result += `previousRestrictionType: ${this._previousRestrictionType.toString()}`;
		result += `newRestrictionType: ${this._newRestrictionType.toString()}`;
		result += ')';
		return result;
	}
}

class EmbeddedMosaicGlobalRestrictionTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_GLOBAL_RESTRICTION;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		mosaicId: 'pod:UnresolvedMosaicId',
		referenceMosaicId: 'pod:UnresolvedMosaicId',
		previousRestrictionType: 'enum:MosaicRestrictionType',
		newRestrictionType: 'enum:MosaicRestrictionType'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedMosaicGlobalRestrictionTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicGlobalRestrictionTransaction.TRANSACTION_TYPE;
		this._mosaicId = new UnresolvedMosaicId();
		this._referenceMosaicId = new UnresolvedMosaicId();
		this._restrictionKey = 0n;
		this._previousRestrictionValue = 0n;
		this._newRestrictionValue = 0n;
		this._previousRestrictionType = MosaicRestrictionType.NONE;
		this._newRestrictionType = MosaicRestrictionType.NONE;
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	get referenceMosaicId() {
		return this._referenceMosaicId;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	get previousRestrictionType() {
		return this._previousRestrictionType;
	}

	get newRestrictionType() {
		return this._newRestrictionType;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	set referenceMosaicId(value) {
		this._referenceMosaicId = value;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	set previousRestrictionType(value) {
		this._previousRestrictionType = value;
	}

	set newRestrictionType(value) {
		this._newRestrictionType = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.mosaicId.size;
		size += this.referenceMosaicId.size;
		size += 8;
		size += 8;
		size += 8;
		size += this.previousRestrictionType.size;
		size += this.newRestrictionType.size;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(mosaicId.size);
		const referenceMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shift(referenceMosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shift(8);
		const previousRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shift(previousRestrictionType.size);
		const newRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shift(newRestrictionType.size);

		const instance = new EmbeddedMosaicGlobalRestrictionTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._mosaicId = mosaicId;
		instance._referenceMosaicId = referenceMosaicId;
		instance._restrictionKey = restrictionKey;
		instance._previousRestrictionValue = previousRestrictionValue;
		instance._newRestrictionValue = newRestrictionValue;
		instance._previousRestrictionType = previousRestrictionType;
		instance._newRestrictionType = newRestrictionType;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._mosaicId.serialize());
		buffer_.write(this._referenceMosaicId.serialize());
		buffer_.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer_.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer_.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer_.write(this._previousRestrictionType.serialize());
		buffer_.write(this._newRestrictionType.serialize());
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `mosaicId: ${this._mosaicId.toString()}`;
		result += `referenceMosaicId: ${this._referenceMosaicId.toString()}`;
		result += `restrictionKey: ${"0x".concat(this._restrictionKey.toString(16))}`;
		result += `previousRestrictionValue: ${"0x".concat(this._previousRestrictionValue.toString(16))}`;
		result += `newRestrictionValue: ${"0x".concat(this._newRestrictionValue.toString(16))}`;
		result += `previousRestrictionType: ${this._previousRestrictionType.toString()}`;
		result += `newRestrictionType: ${this._newRestrictionType.toString()}`;
		result += ')';
		return result;
	}
}

class TransferTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.TRANSFER;

	static TYPE_HINTS = {
		signature: 'pod:Signature',
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:UnresolvedAddress',
		mosaics: 'array[UnresolvedMosaic]',
		message: 'bytes_array'
	};

	constructor() {
		this._signature = new Signature();
		this._signerPublicKey = new PublicKey();
		this._version = TransferTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = TransferTransaction.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new UnresolvedAddress();
		this._mosaics = [];
		this._message = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._transferTransactionBodyReserved_1 = 0;  // reserved field
		this._transferTransactionBodyReserved_2 = 0;  // reserved field
	}

	get signature() {
		return this._signature;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get fee() {
		return this._fee;
	}

	get deadline() {
		return this._deadline;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	get mosaics() {
		return this._mosaics;
	}

	get message() {
		return this._message;
	}

	set signature(value) {
		this._signature = value;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set fee(value) {
		this._fee = value;
	}

	set deadline(value) {
		this._deadline = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	set mosaics(value) {
		this._mosaics = value;
	}

	set message(value) {
		this._message = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signature.size;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.recipientAddress.size;
		size += 2;
		size += 1;
		size += 1;
		size += 4;
		size += this.mosaics.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this._message.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (verifiableEntityHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shift(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shift(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shift(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(recipientAddress.size);
		const messageSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const mosaicsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const transferTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		if (transferTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_1})`);
		const transferTransactionBodyReserved_2 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (transferTransactionBodyReserved_2 !== 0)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_2})`);
		const mosaics = arrayHelpers.read_array_count(view.buffer, UnresolvedMosaic, mosaicsCount, e => e.mosaicId.value);
		view.shift(mosaics.map(e => e.size).reduce((a, b) => a + b, 0));
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shift(messageSize);

		const instance = new TransferTransaction();
		instance._signature = signature;
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._mosaics = mosaics;
		instance._message = message;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer_.write(this._signature.serialize());
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._fee.serialize());
		buffer_.write(this._deadline.serialize());
		buffer_.write(this._recipientAddress.serialize());
		buffer_.write(converter.intToBytes(this._message.length, 2, false)); /* bound: message_size */ 
		buffer_.write(converter.intToBytes(this._mosaics.length, 1, false)); /* bound: mosaics_count */ 
		buffer_.write(converter.intToBytes(this._transferTransactionBodyReserved_1, 1, false));
		buffer_.write(converter.intToBytes(this._transferTransactionBodyReserved_2, 4, false));
		arrayHelpers.write_array(buffer_, this._mosaics, e => e.mosaicId.value);
		buffer_.write(this._message);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}`;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `fee: ${this._fee.toString()}`;
		result += `deadline: ${this._deadline.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += `mosaics: ${this._mosaics.map(e => e.toString())}`;
		result += `message: ${this._message /* FIXME */}`;
		result += ')';
		return result;
	}
}

class EmbeddedTransferTransaction {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.TRANSFER;

	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		network: 'enum:NetworkType',
		type: 'enum:TransactionType',
		recipientAddress: 'pod:UnresolvedAddress',
		mosaics: 'array[UnresolvedMosaic]',
		message: 'bytes_array'
	};

	constructor() {
		this._signerPublicKey = new PublicKey();
		this._version = EmbeddedTransferTransaction.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedTransferTransaction.TRANSACTION_TYPE;
		this._recipientAddress = new UnresolvedAddress();
		this._mosaics = [];
		this._message = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0;  // reserved field
		this._entityBodyReserved_1 = 0;  // reserved field
		this._transferTransactionBodyReserved_1 = 0;  // reserved field
		this._transferTransactionBodyReserved_2 = 0;  // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	get version() {
		return this._version;
	}

	get network() {
		return this._network;
	}

	get type() {
		return this._type;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	get mosaics() {
		return this._mosaics;
	}

	get message() {
		return this._message;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	set version(value) {
		this._version = value;
	}

	set network(value) {
		this._network = value;
	}

	set type(value) {
		this._type = value;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	set mosaics(value) {
		this._mosaics = value;
	}

	set message(value) {
		this._message = value;
	}

	get size() {
		let size = 0
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.recipientAddress.size;
		size += 2;
		size += 1;
		size += 1;
		size += 4;
		size += this.mosaics.map(e => e.size).reduce((a, b) => a + b, 0);
		size += this._message.length;
		return size
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		view.shrink(size - 4);
		/* size = undefined; */
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (embeddedTransactionHeaderReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shift(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (entityBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shift(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shift(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shift(recipientAddress.size);
		const messageSize = converter.bytesToInt(view.buffer, 2, false);
		view.shift(2);
		const mosaicsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		const transferTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 1, false);
		view.shift(1);
		if (transferTransactionBodyReserved_1 !== 0)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_1})`);
		const transferTransactionBodyReserved_2 = converter.bytesToInt(view.buffer, 4, false);
		view.shift(4);
		if (transferTransactionBodyReserved_2 !== 0)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_2})`);
		const mosaics = arrayHelpers.read_array_count(view.buffer, UnresolvedMosaic, mosaicsCount, e => e.mosaicId.value);
		view.shift(mosaics.map(e => e.size).reduce((a, b) => a + b, 0));
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shift(messageSize);

		const instance = new EmbeddedTransferTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		instance._recipientAddress = recipientAddress;
		instance._mosaics = mosaics;
		instance._message = message;
		return instance;
	}

	serialize() {
		const buffer_ = new Writer(this.size);
		buffer_.write(converter.intToBytes(this.size, 4, false));
		buffer_.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer_.write(this._signerPublicKey.serialize());
		buffer_.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer_.write(converter.intToBytes(this._version, 1, false));
		buffer_.write(this._network.serialize());
		buffer_.write(this._type.serialize());
		buffer_.write(this._recipientAddress.serialize());
		buffer_.write(converter.intToBytes(this._message.length, 2, false)); /* bound: message_size */ 
		buffer_.write(converter.intToBytes(this._mosaics.length, 1, false)); /* bound: mosaics_count */ 
		buffer_.write(converter.intToBytes(this._transferTransactionBodyReserved_1, 1, false));
		buffer_.write(converter.intToBytes(this._transferTransactionBodyReserved_2, 4, false));
		arrayHelpers.write_array(buffer_, this._mosaics, e => e.mosaicId.value);
		buffer_.write(this._message);
		return buffer_.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}`;
		result += `version: ${"0x".concat(this._version.toString(16))}`;
		result += `network: ${this._network.toString()}`;
		result += `type: ${this._type.toString()}`;
		result += `recipientAddress: ${this._recipientAddress.toString()}`;
		result += `mosaics: ${this._mosaics.map(e => e.toString())}`;
		result += `message: ${this._message /* FIXME */}`;
		result += ')';
		return result;
	}
}

class TransactionFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = Transaction.deserialize(view.buffer);
		const mapping = new Map([
			[TransactionFactory.toKey([AccountKeyLinkTransaction.TRANSACTION_TYPE.value]), AccountKeyLinkTransaction],
			[TransactionFactory.toKey([NodeKeyLinkTransaction.TRANSACTION_TYPE.value]), NodeKeyLinkTransaction],
			[TransactionFactory.toKey([AggregateCompleteTransaction.TRANSACTION_TYPE.value]), AggregateCompleteTransaction],
			[TransactionFactory.toKey([AggregateBondedTransaction.TRANSACTION_TYPE.value]), AggregateBondedTransaction],
			[TransactionFactory.toKey([VotingKeyLinkTransaction.TRANSACTION_TYPE.value]), VotingKeyLinkTransaction],
			[TransactionFactory.toKey([VrfKeyLinkTransaction.TRANSACTION_TYPE.value]), VrfKeyLinkTransaction],
			[TransactionFactory.toKey([HashLockTransaction.TRANSACTION_TYPE.value]), HashLockTransaction],
			[TransactionFactory.toKey([SecretLockTransaction.TRANSACTION_TYPE.value]), SecretLockTransaction],
			[TransactionFactory.toKey([SecretProofTransaction.TRANSACTION_TYPE.value]), SecretProofTransaction],
			[TransactionFactory.toKey([AccountMetadataTransaction.TRANSACTION_TYPE.value]), AccountMetadataTransaction],
			[TransactionFactory.toKey([MosaicMetadataTransaction.TRANSACTION_TYPE.value]), MosaicMetadataTransaction],
			[TransactionFactory.toKey([NamespaceMetadataTransaction.TRANSACTION_TYPE.value]), NamespaceMetadataTransaction],
			[TransactionFactory.toKey([MosaicDefinitionTransaction.TRANSACTION_TYPE.value]), MosaicDefinitionTransaction],
			[TransactionFactory.toKey([MosaicSupplyChangeTransaction.TRANSACTION_TYPE.value]), MosaicSupplyChangeTransaction],
			[TransactionFactory.toKey([MosaicSupplyRevocationTransaction.TRANSACTION_TYPE.value]), MosaicSupplyRevocationTransaction],
			[TransactionFactory.toKey([MultisigAccountModificationTransaction.TRANSACTION_TYPE.value]), MultisigAccountModificationTransaction],
			[TransactionFactory.toKey([AddressAliasTransaction.TRANSACTION_TYPE.value]), AddressAliasTransaction],
			[TransactionFactory.toKey([MosaicAliasTransaction.TRANSACTION_TYPE.value]), MosaicAliasTransaction],
			[TransactionFactory.toKey([NamespaceRegistrationTransaction.TRANSACTION_TYPE.value]), NamespaceRegistrationTransaction],
			[TransactionFactory.toKey([AccountAddressRestrictionTransaction.TRANSACTION_TYPE.value]), AccountAddressRestrictionTransaction],
			[TransactionFactory.toKey([AccountMosaicRestrictionTransaction.TRANSACTION_TYPE.value]), AccountMosaicRestrictionTransaction],
			[TransactionFactory.toKey([AccountOperationRestrictionTransaction.TRANSACTION_TYPE.value]), AccountOperationRestrictionTransaction],
			[TransactionFactory.toKey([MosaicAddressRestrictionTransaction.TRANSACTION_TYPE.value]), MosaicAddressRestrictionTransaction],
			[TransactionFactory.toKey([MosaicGlobalRestrictionTransaction.TRANSACTION_TYPE.value]), MosaicGlobalRestrictionTransaction],
			[TransactionFactory.toKey([TransferTransaction.TRANSACTION_TYPE.value]), TransferTransaction]
		]);
		const discriminator = TransactionFactory.toKey([parent.type.value]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			"account_key_link_transaction": AccountKeyLinkTransaction,
			"node_key_link_transaction": NodeKeyLinkTransaction,
			"aggregate_complete_transaction": AggregateCompleteTransaction,
			"aggregate_bonded_transaction": AggregateBondedTransaction,
			"voting_key_link_transaction": VotingKeyLinkTransaction,
			"vrf_key_link_transaction": VrfKeyLinkTransaction,
			"hash_lock_transaction": HashLockTransaction,
			"secret_lock_transaction": SecretLockTransaction,
			"secret_proof_transaction": SecretProofTransaction,
			"account_metadata_transaction": AccountMetadataTransaction,
			"mosaic_metadata_transaction": MosaicMetadataTransaction,
			"namespace_metadata_transaction": NamespaceMetadataTransaction,
			"mosaic_definition_transaction": MosaicDefinitionTransaction,
			"mosaic_supply_change_transaction": MosaicSupplyChangeTransaction,
			"mosaic_supply_revocation_transaction": MosaicSupplyRevocationTransaction,
			"multisig_account_modification_transaction": MultisigAccountModificationTransaction,
			"address_alias_transaction": AddressAliasTransaction,
			"mosaic_alias_transaction": MosaicAliasTransaction,
			"namespace_registration_transaction": NamespaceRegistrationTransaction,
			"account_address_restriction_transaction": AccountAddressRestrictionTransaction,
			"account_mosaic_restriction_transaction": AccountMosaicRestrictionTransaction,
			"account_operation_restriction_transaction": AccountOperationRestrictionTransaction,
			"mosaic_address_restriction_transaction": MosaicAddressRestrictionTransaction,
			"mosaic_global_restriction_transaction": MosaicGlobalRestrictionTransaction,
			"transfer_transaction": TransferTransaction
		};

		if (!mapping.hasOwnProperty(entityName))
			throw RangeError('unknown Transaction type')

		return new mapping[entityName]()
	}
}

class EmbeddedTransactionFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = EmbeddedTransaction.deserialize(view.buffer);
		const mapping = new Map([
			[EmbeddedTransactionFactory.toKey([EmbeddedAccountKeyLinkTransaction.TRANSACTION_TYPE.value]), EmbeddedAccountKeyLinkTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedNodeKeyLinkTransaction.TRANSACTION_TYPE.value]), EmbeddedNodeKeyLinkTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedVotingKeyLinkTransaction.TRANSACTION_TYPE.value]), EmbeddedVotingKeyLinkTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedVrfKeyLinkTransaction.TRANSACTION_TYPE.value]), EmbeddedVrfKeyLinkTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedHashLockTransaction.TRANSACTION_TYPE.value]), EmbeddedHashLockTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedSecretLockTransaction.TRANSACTION_TYPE.value]), EmbeddedSecretLockTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedSecretProofTransaction.TRANSACTION_TYPE.value]), EmbeddedSecretProofTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedAccountMetadataTransaction.TRANSACTION_TYPE.value]), EmbeddedAccountMetadataTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicMetadataTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicMetadataTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedNamespaceMetadataTransaction.TRANSACTION_TYPE.value]), EmbeddedNamespaceMetadataTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicDefinitionTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicDefinitionTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicSupplyChangeTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicSupplyChangeTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicSupplyRevocationTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicSupplyRevocationTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMultisigAccountModificationTransaction.TRANSACTION_TYPE.value]), EmbeddedMultisigAccountModificationTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedAddressAliasTransaction.TRANSACTION_TYPE.value]), EmbeddedAddressAliasTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicAliasTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicAliasTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedNamespaceRegistrationTransaction.TRANSACTION_TYPE.value]), EmbeddedNamespaceRegistrationTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedAccountAddressRestrictionTransaction.TRANSACTION_TYPE.value]), EmbeddedAccountAddressRestrictionTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedAccountMosaicRestrictionTransaction.TRANSACTION_TYPE.value]), EmbeddedAccountMosaicRestrictionTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedAccountOperationRestrictionTransaction.TRANSACTION_TYPE.value]), EmbeddedAccountOperationRestrictionTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicAddressRestrictionTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicAddressRestrictionTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedMosaicGlobalRestrictionTransaction.TRANSACTION_TYPE.value]), EmbeddedMosaicGlobalRestrictionTransaction],
			[EmbeddedTransactionFactory.toKey([EmbeddedTransferTransaction.TRANSACTION_TYPE.value]), EmbeddedTransferTransaction]
		]);
		const discriminator = EmbeddedTransactionFactory.toKey([parent.type.value]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			"account_key_link_transaction": EmbeddedAccountKeyLinkTransaction,
			"node_key_link_transaction": EmbeddedNodeKeyLinkTransaction,
			"voting_key_link_transaction": EmbeddedVotingKeyLinkTransaction,
			"vrf_key_link_transaction": EmbeddedVrfKeyLinkTransaction,
			"hash_lock_transaction": EmbeddedHashLockTransaction,
			"secret_lock_transaction": EmbeddedSecretLockTransaction,
			"secret_proof_transaction": EmbeddedSecretProofTransaction,
			"account_metadata_transaction": EmbeddedAccountMetadataTransaction,
			"mosaic_metadata_transaction": EmbeddedMosaicMetadataTransaction,
			"namespace_metadata_transaction": EmbeddedNamespaceMetadataTransaction,
			"mosaic_definition_transaction": EmbeddedMosaicDefinitionTransaction,
			"mosaic_supply_change_transaction": EmbeddedMosaicSupplyChangeTransaction,
			"mosaic_supply_revocation_transaction": EmbeddedMosaicSupplyRevocationTransaction,
			"multisig_account_modification_transaction": EmbeddedMultisigAccountModificationTransaction,
			"address_alias_transaction": EmbeddedAddressAliasTransaction,
			"mosaic_alias_transaction": EmbeddedMosaicAliasTransaction,
			"namespace_registration_transaction": EmbeddedNamespaceRegistrationTransaction,
			"account_address_restriction_transaction": EmbeddedAccountAddressRestrictionTransaction,
			"account_mosaic_restriction_transaction": EmbeddedAccountMosaicRestrictionTransaction,
			"account_operation_restriction_transaction": EmbeddedAccountOperationRestrictionTransaction,
			"mosaic_address_restriction_transaction": EmbeddedMosaicAddressRestrictionTransaction,
			"mosaic_global_restriction_transaction": EmbeddedMosaicGlobalRestrictionTransaction,
			"transfer_transaction": EmbeddedTransferTransaction
		};

		if (!mapping.hasOwnProperty(entityName))
			throw RangeError('unknown EmbeddedTransaction type')

		return new mapping[entityName]()
	}
}

class BlockFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = Block.deserialize(view.buffer);
		const mapping = new Map([
			[BlockFactory.toKey([NemesisBlock.BLOCK_TYPE.value]), NemesisBlock],
			[BlockFactory.toKey([NormalBlock.BLOCK_TYPE.value]), NormalBlock],
			[BlockFactory.toKey([ImportanceBlock.BLOCK_TYPE.value]), ImportanceBlock]
		]);
		const discriminator = BlockFactory.toKey([parent.type.value]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			"nemesis_block": NemesisBlock,
			"normal_block": NormalBlock,
			"importance_block": ImportanceBlock
		};

		if (!mapping.hasOwnProperty(entityName))
			throw RangeError('unknown Block type')

		return new mapping[entityName]()
	}
}

class ReceiptFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = Receipt.deserialize(view.buffer);
		const mapping = new Map([
			[ReceiptFactory.toKey([HarvestFeeReceipt.RECEIPT_TYPE.value]), HarvestFeeReceipt],
			[ReceiptFactory.toKey([InflationReceipt.RECEIPT_TYPE.value]), InflationReceipt],
			[ReceiptFactory.toKey([LockHashCreatedFeeReceipt.RECEIPT_TYPE.value]), LockHashCreatedFeeReceipt],
			[ReceiptFactory.toKey([LockHashCompletedFeeReceipt.RECEIPT_TYPE.value]), LockHashCompletedFeeReceipt],
			[ReceiptFactory.toKey([LockHashExpiredFeeReceipt.RECEIPT_TYPE.value]), LockHashExpiredFeeReceipt],
			[ReceiptFactory.toKey([LockSecretCreatedFeeReceipt.RECEIPT_TYPE.value]), LockSecretCreatedFeeReceipt],
			[ReceiptFactory.toKey([LockSecretCompletedFeeReceipt.RECEIPT_TYPE.value]), LockSecretCompletedFeeReceipt],
			[ReceiptFactory.toKey([LockSecretExpiredFeeReceipt.RECEIPT_TYPE.value]), LockSecretExpiredFeeReceipt],
			[ReceiptFactory.toKey([MosaicExpiredReceipt.RECEIPT_TYPE.value]), MosaicExpiredReceipt],
			[ReceiptFactory.toKey([MosaicRentalFeeReceipt.RECEIPT_TYPE.value]), MosaicRentalFeeReceipt],
			[ReceiptFactory.toKey([NamespaceExpiredReceipt.RECEIPT_TYPE.value]), NamespaceExpiredReceipt],
			[ReceiptFactory.toKey([NamespaceDeletedReceipt.RECEIPT_TYPE.value]), NamespaceDeletedReceipt],
			[ReceiptFactory.toKey([NamespaceRentalFeeReceipt.RECEIPT_TYPE.value]), NamespaceRentalFeeReceipt]
		]);
		const discriminator = ReceiptFactory.toKey([parent.type.value]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			"harvest_fee_receipt": HarvestFeeReceipt,
			"inflation_receipt": InflationReceipt,
			"lock_hash_created_fee_receipt": LockHashCreatedFeeReceipt,
			"lock_hash_completed_fee_receipt": LockHashCompletedFeeReceipt,
			"lock_hash_expired_fee_receipt": LockHashExpiredFeeReceipt,
			"lock_secret_created_fee_receipt": LockSecretCreatedFeeReceipt,
			"lock_secret_completed_fee_receipt": LockSecretCompletedFeeReceipt,
			"lock_secret_expired_fee_receipt": LockSecretExpiredFeeReceipt,
			"mosaic_expired_receipt": MosaicExpiredReceipt,
			"mosaic_rental_fee_receipt": MosaicRentalFeeReceipt,
			"namespace_expired_receipt": NamespaceExpiredReceipt,
			"namespace_deleted_receipt": NamespaceDeletedReceipt,
			"namespace_rental_fee_receipt": NamespaceRentalFeeReceipt
		};

		if (!mapping.hasOwnProperty(entityName))
			throw RangeError('unknown Receipt type')

		return new mapping[entityName]()
	}
}

module.exports = {
	Amount, BlockDuration, BlockFeeMultiplier, Difficulty, FinalizationEpoch, FinalizationPoint, Height, Importance, ImportanceHeight, 
	UnresolvedMosaicId, MosaicId, Timestamp, UnresolvedAddress, Address, Hash256, Hash512, PublicKey, VotingPublicKey, Signature, Mosaic, 
	UnresolvedMosaic, LinkAction, NetworkType, TransactionType, Transaction, EmbeddedTransaction, ProofGamma, ProofVerificationHash, 
	ProofScalar, BlockType, VrfProof, Block, NemesisBlock, NormalBlock, ImportanceBlock, FinalizationRound, FinalizedBlockHeader, ReceiptType, 
	Receipt, HarvestFeeReceipt, InflationReceipt, LockHashCreatedFeeReceipt, LockHashCompletedFeeReceipt, LockHashExpiredFeeReceipt, 
	LockSecretCreatedFeeReceipt, LockSecretCompletedFeeReceipt, LockSecretExpiredFeeReceipt, MosaicExpiredReceipt, MosaicRentalFeeReceipt, 
	NamespaceId, NamespaceRegistrationType, AliasAction, NamespaceExpiredReceipt, NamespaceDeletedReceipt, NamespaceRentalFeeReceipt, 
	ReceiptSource, AddressResolutionEntry, AddressResolutionStatement, MosaicResolutionEntry, MosaicResolutionStatement, TransactionStatement, 
	BlockStatement, AccountType, AccountKeyTypeFlags, AccountStateFormat, PinnedVotingKey, ImportanceSnapshot, HeightActivityBucket, 
	HeightActivityBuckets, AccountState, LockStatus, HashLockInfo, ScopedMetadataKey, MetadataType, MetadataValue, MetadataEntry, MosaicNonce, 
	MosaicFlags, MosaicSupplyChangeAction, MosaicProperties, MosaicDefinition, MosaicEntry, MultisigEntry, NamespaceLifetime, 
	NamespaceAliasType, NamespaceAlias, NamespacePath, RootNamespaceHistory, AccountRestrictionFlags, AccountRestrictionAddressValue, 
	AccountRestrictionMosaicValue, AccountRestrictionTransactionTypeValue, AccountRestrictionsInfo, AccountRestrictions, MosaicRestrictionKey, 
	MosaicRestrictionType, MosaicRestrictionEntryType, AddressKeyValue, AddressKeyValueSet, RestrictionRule, GlobalKeyValue, GlobalKeyValueSet, 
	MosaicAddressRestrictionEntry, MosaicGlobalRestrictionEntry, MosaicRestrictionEntry, LockHashAlgorithm, SecretLockInfo, 
	AccountKeyLinkTransaction, EmbeddedAccountKeyLinkTransaction, NodeKeyLinkTransaction, EmbeddedNodeKeyLinkTransaction, Cosignature, 
	DetachedCosignature, AggregateCompleteTransaction, AggregateBondedTransaction, VotingKeyLinkTransaction, EmbeddedVotingKeyLinkTransaction, 
	VrfKeyLinkTransaction, EmbeddedVrfKeyLinkTransaction, HashLockTransaction, EmbeddedHashLockTransaction, SecretLockTransaction, 
	EmbeddedSecretLockTransaction, SecretProofTransaction, EmbeddedSecretProofTransaction, AccountMetadataTransaction, 
	EmbeddedAccountMetadataTransaction, MosaicMetadataTransaction, EmbeddedMosaicMetadataTransaction, NamespaceMetadataTransaction, 
	EmbeddedNamespaceMetadataTransaction, MosaicDefinitionTransaction, EmbeddedMosaicDefinitionTransaction, MosaicSupplyChangeTransaction, 
	EmbeddedMosaicSupplyChangeTransaction, MosaicSupplyRevocationTransaction, EmbeddedMosaicSupplyRevocationTransaction, 
	MultisigAccountModificationTransaction, EmbeddedMultisigAccountModificationTransaction, AddressAliasTransaction, 
	EmbeddedAddressAliasTransaction, MosaicAliasTransaction, EmbeddedMosaicAliasTransaction, NamespaceRegistrationTransaction, 
	EmbeddedNamespaceRegistrationTransaction, AccountAddressRestrictionTransaction, EmbeddedAccountAddressRestrictionTransaction, 
	AccountMosaicRestrictionTransaction, EmbeddedAccountMosaicRestrictionTransaction, AccountOperationRestrictionTransaction, 
	EmbeddedAccountOperationRestrictionTransaction, MosaicAddressRestrictionTransaction, EmbeddedMosaicAddressRestrictionTransaction, 
	MosaicGlobalRestrictionTransaction, EmbeddedMosaicGlobalRestrictionTransaction, TransferTransaction, EmbeddedTransferTransaction, 
	TransactionFactory, EmbeddedTransactionFactory, BlockFactory, ReceiptFactory
}