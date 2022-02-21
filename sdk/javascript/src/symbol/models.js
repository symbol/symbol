/* eslint-disable max-len, object-property-newline, no-underscore-dangle, no-use-before-define */

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

	get size() { // eslint-disable-line class-methods-use-this
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

	get size() { // eslint-disable-line class-methods-use-this
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

	get size() { // eslint-disable-line class-methods-use-this
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

	get size() { // eslint-disable-line class-methods-use-this
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

	get size() { // eslint-disable-line class-methods-use-this
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

	get size() { // eslint-disable-line class-methods-use-this
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

	get size() { // eslint-disable-line class-methods-use-this
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

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get amount() {
		return this._amount;
	}

	set amount(value) {
		this._amount = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.mosaicId.size;
		size += this.amount.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const amount = Amount.deserialize(view.buffer);
		view.shiftRight(amount.size);

		const instance = new Mosaic();
		instance._mosaicId = mosaicId;
		instance._amount = amount;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._amount.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `amount: ${this._amount.toString()}, `;
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

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get amount() {
		return this._amount;
	}

	set amount(value) {
		this._amount = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.mosaicId.size;
		size += this.amount.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const amount = Amount.deserialize(view.buffer);
		view.shiftRight(amount.size);

		const instance = new UnresolvedMosaic();
		instance._mosaicId = mosaicId;
		instance._amount = amount;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._amount.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `amount: ${this._amount.toString()}, `;
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

	static valueToKey(value) {
		const values = [
			0, 1
		];
		const keys = [
			'UNLINK', 'LINK'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return LinkAction[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `LinkAction.${LinkAction.valueToKey(this.value)}`;
	}
}

class NetworkType {
	static MAINNET = new NetworkType(104);

	static TESTNET = new NetworkType(152);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			104, 152
		];
		const keys = [
			'MAINNET', 'TESTNET'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return NetworkType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `NetworkType.${NetworkType.valueToKey(this.value)}`;
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

	static valueToKey(value) {
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

		return keys[index];
	}

	static fromValue(value) {
		return TransactionType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}

	toString() {
		return `TransactionType.${TransactionType.valueToKey(this.value)}`;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);

		const instance = new EmbeddedTransaction();
		instance._signerPublicKey = signerPublicKey;
		instance._version = version;
		instance._network = network;
		instance._type = type;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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

	set version(value) {
		this._version = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 8;
		size += this.signerPublicKey.size;
		size += this.signature.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);

		const instance = new Cosignature();
		instance._version = version;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._version, 8, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._signature.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
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

	set version(value) {
		this._version = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get parentHash() {
		return this._parentHash;
	}

	set parentHash(value) {
		this._parentHash = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 8;
		size += this.signerPublicKey.size;
		size += this.signature.size;
		size += this.parentHash.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const version = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const parentHash = Hash256.deserialize(view.buffer);
		view.shiftRight(parentHash.size);

		const instance = new DetachedCosignature();
		instance._version = version;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._parentHash = parentHash;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._version, 8, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._signature.serialize());
		buffer.write(this._parentHash.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `parentHash: ${this._parentHash.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	get transactions() {
		return this._transactions;
	}

	set transactions(value) {
		this._transactions = value;
	}

	get cosignatures() {
		return this._cosignatures;
	}

	set cosignatures(value) {
		this._cosignatures = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.transactions, 8, false);
		size += arrayHelpers.size(this.cosignatures);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(transactionsHash.size);
		const payloadSize = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		const aggregateTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== aggregateTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${aggregateTransactionHeaderReserved_1})`);
		const transactions = arrayHelpers.readVariableSizeElements(view.window(payloadSize), EmbeddedTransactionFactory, 8, false);
		view.shiftRight(payloadSize);
		const cosignatures = arrayHelpers.readArray(view.buffer, Cosignature);
		view.shiftRight(arrayHelpers.size(cosignatures));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._transactionsHash.serialize());
		buffer.write(converter.intToBytes(arrayHelpers.size(this.transactions, 8, false), 4, false)); // bound: payload_size
		buffer.write(converter.intToBytes(this._aggregateTransactionHeaderReserved_1, 4, false));
		arrayHelpers.writeVariableSizeElements(buffer, this._transactions, 8, false);
		arrayHelpers.writeArray(buffer, this._cosignatures);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `transactionsHash: ${this._transactionsHash.toString()}, `;
		result += `transactions: [${this._transactions.map(e => e.toString()).join(',')}], `;
		result += `cosignatures: [${this._cosignatures.map(e => e.toString()).join(',')}], `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	get transactions() {
		return this._transactions;
	}

	set transactions(value) {
		this._transactions = value;
	}

	get cosignatures() {
		return this._cosignatures;
	}

	set cosignatures(value) {
		this._cosignatures = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.transactions, 8, false);
		size += arrayHelpers.size(this.cosignatures);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(transactionsHash.size);
		const payloadSize = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		const aggregateTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== aggregateTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${aggregateTransactionHeaderReserved_1})`);
		const transactions = arrayHelpers.readVariableSizeElements(view.window(payloadSize), EmbeddedTransactionFactory, 8, false);
		view.shiftRight(payloadSize);
		const cosignatures = arrayHelpers.readArray(view.buffer, Cosignature);
		view.shiftRight(arrayHelpers.size(cosignatures));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._transactionsHash.serialize());
		buffer.write(converter.intToBytes(arrayHelpers.size(this.transactions, 8, false), 4, false)); // bound: payload_size
		buffer.write(converter.intToBytes(this._aggregateTransactionHeaderReserved_1, 4, false));
		arrayHelpers.writeVariableSizeElements(buffer, this._transactions, 8, false);
		arrayHelpers.writeArray(buffer, this._cosignatures);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `transactionsHash: ${this._transactionsHash.toString()}, `;
		result += `transactions: [${this._transactions.map(e => e.toString()).join(',')}], `;
		result += `cosignatures: [${this._cosignatures.map(e => e.toString()).join(',')}], `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get startEpoch() {
		return this._startEpoch;
	}

	set startEpoch(value) {
		this._startEpoch = value;
	}

	get endEpoch() {
		return this._endEpoch;
	}

	set endEpoch(value) {
		this._endEpoch = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = VotingPublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const startEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shiftRight(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shiftRight(endEpoch.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._startEpoch.serialize());
		buffer.write(this._endEpoch.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `startEpoch: ${this._startEpoch.toString()}, `;
		result += `endEpoch: ${this._endEpoch.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get startEpoch() {
		return this._startEpoch;
	}

	set startEpoch(value) {
		this._startEpoch = value;
	}

	get endEpoch() {
		return this._endEpoch;
	}

	set endEpoch(value) {
		this._endEpoch = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = VotingPublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const startEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shiftRight(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserialize(view.buffer);
		view.shiftRight(endEpoch.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._startEpoch.serialize());
		buffer.write(this._endEpoch.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `startEpoch: ${this._startEpoch.toString()}, `;
		result += `endEpoch: ${this._endEpoch.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get linkedPublicKey() {
		return this._linkedPublicKey;
	}

	set linkedPublicKey(value) {
		this._linkedPublicKey = value;
	}

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.linkedPublicKey.size;
		size += this.linkAction.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._linkedPublicKey.serialize());
		buffer.write(this._linkAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `linkedPublicKey: ${this._linkedPublicKey.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get mosaic() {
		return this._mosaic;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get hash() {
		return this._hash;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shiftRight(duration.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shiftRight(hash.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._duration.serialize());
		buffer.write(this._hash.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `duration: ${this._duration.toString()}, `;
		result += `hash: ${this._hash.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get mosaic() {
		return this._mosaic;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get hash() {
		return this._hash;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shiftRight(duration.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shiftRight(hash.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._duration.serialize());
		buffer.write(this._hash.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `duration: ${this._duration.toString()}, `;
		result += `hash: ${this._hash.toString()}, `;
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

	static valueToKey(value) {
		const values = [
			0, 1, 2
		];
		const keys = [
			'SHA3_256', 'HASH_160', 'HASH_256'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return LockHashAlgorithm[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `LockHashAlgorithm.${LockHashAlgorithm.valueToKey(this.value)}`;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get secret() {
		return this._secret;
	}

	set secret(value) {
		this._secret = value;
	}

	get mosaic() {
		return this._mosaic;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shiftRight(duration.size);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shiftRight(hashAlgorithm.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._secret.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._duration.serialize());
		buffer.write(this._hashAlgorithm.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `secret: ${this._secret.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `duration: ${this._duration.toString()}, `;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get secret() {
		return this._secret;
	}

	set secret(value) {
		this._secret = value;
	}

	get mosaic() {
		return this._mosaic;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shiftRight(duration.size);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shiftRight(hashAlgorithm.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._secret.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._duration.serialize());
		buffer.write(this._hashAlgorithm.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `secret: ${this._secret.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `duration: ${this._duration.toString()}, `;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get secret() {
		return this._secret;
	}

	set secret(value) {
		this._secret = value;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	get proof() {
		return this._proof;
	}

	set proof(value) {
		this._proof = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const proofSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shiftRight(hashAlgorithm.size);
		const proof = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, proofSize);
		view.shiftRight(proofSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._secret.serialize());
		buffer.write(converter.intToBytes(this._proof.length, 2, false)); // bound: proof_size
		buffer.write(this._hashAlgorithm.serialize());
		buffer.write(this._proof);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `secret: ${this._secret.toString()}, `;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}, `;
		result += `proof: hex(${converter.uint8ToHex(this._proof)}), `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get secret() {
		return this._secret;
	}

	set secret(value) {
		this._secret = value;
	}

	get hashAlgorithm() {
		return this._hashAlgorithm;
	}

	set hashAlgorithm(value) {
		this._hashAlgorithm = value;
	}

	get proof() {
		return this._proof;
	}

	set proof(value) {
		this._proof = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const proofSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const hashAlgorithm = LockHashAlgorithm.deserialize(view.buffer);
		view.shiftRight(hashAlgorithm.size);
		const proof = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, proofSize);
		view.shiftRight(proofSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._secret.serialize());
		buffer.write(converter.intToBytes(this._proof.length, 2, false)); // bound: proof_size
		buffer.write(this._hashAlgorithm.serialize());
		buffer.write(this._proof);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `secret: ${this._secret.toString()}, `;
		result += `hashAlgorithm: ${this._hashAlgorithm.toString()}, `;
		result += `proof: hex(${converter.uint8ToHex(this._proof)}), `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	get value() {
		return this._value;
	}

	set value(value) {
		this._value = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._targetAddress.serialize());
		buffer.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer.write(converter.intToBytes(this._value.length, 2, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += `scopedMetadataKey: ${'0x'.concat(this._scopedMetadataKey.toString(16))}, `;
		result += `valueSizeDelta: ${'0x'.concat(this._valueSizeDelta.toString(16))}, `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	get value() {
		return this._value;
	}

	set value(value) {
		this._value = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._targetAddress.serialize());
		buffer.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer.write(converter.intToBytes(this._value.length, 2, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += `scopedMetadataKey: ${'0x'.concat(this._scopedMetadataKey.toString(16))}, `;
		result += `valueSizeDelta: ${'0x'.concat(this._valueSizeDelta.toString(16))}, `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	get targetMosaicId() {
		return this._targetMosaicId;
	}

	set targetMosaicId(value) {
		this._targetMosaicId = value;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	get value() {
		return this._value;
	}

	set value(value) {
		this._value = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(targetMosaicId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._targetAddress.serialize());
		buffer.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer.write(this._targetMosaicId.serialize());
		buffer.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer.write(converter.intToBytes(this._value.length, 2, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += `scopedMetadataKey: ${'0x'.concat(this._scopedMetadataKey.toString(16))}, `;
		result += `targetMosaicId: ${this._targetMosaicId.toString()}, `;
		result += `valueSizeDelta: ${'0x'.concat(this._valueSizeDelta.toString(16))}, `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	get targetMosaicId() {
		return this._targetMosaicId;
	}

	set targetMosaicId(value) {
		this._targetMosaicId = value;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	get value() {
		return this._value;
	}

	set value(value) {
		this._value = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(targetMosaicId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._targetAddress.serialize());
		buffer.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer.write(this._targetMosaicId.serialize());
		buffer.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer.write(converter.intToBytes(this._value.length, 2, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += `scopedMetadataKey: ${'0x'.concat(this._scopedMetadataKey.toString(16))}, `;
		result += `targetMosaicId: ${this._targetMosaicId.toString()}, `;
		result += `valueSizeDelta: ${'0x'.concat(this._valueSizeDelta.toString(16))}, `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
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

	static valueToKey(value) {
		const values = [
			0, 1
		];
		const keys = [
			'ROOT', 'CHILD'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return NamespaceRegistrationType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `NamespaceRegistrationType.${NamespaceRegistrationType.valueToKey(this.value)}`;
	}
}

class AliasAction {
	static UNLINK = new AliasAction(0);

	static LINK = new AliasAction(1);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			0, 1
		];
		const keys = [
			'UNLINK', 'LINK'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return AliasAction[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `AliasAction.${AliasAction.valueToKey(this.value)}`;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	get targetNamespaceId() {
		return this._targetNamespaceId;
	}

	set targetNamespaceId(value) {
		this._targetNamespaceId = value;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	get value() {
		return this._value;
	}

	set value(value) {
		this._value = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetNamespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(targetNamespaceId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._targetAddress.serialize());
		buffer.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer.write(this._targetNamespaceId.serialize());
		buffer.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer.write(converter.intToBytes(this._value.length, 2, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += `scopedMetadataKey: ${'0x'.concat(this._scopedMetadataKey.toString(16))}, `;
		result += `targetNamespaceId: ${this._targetNamespaceId.toString()}, `;
		result += `valueSizeDelta: ${'0x'.concat(this._valueSizeDelta.toString(16))}, `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get scopedMetadataKey() {
		return this._scopedMetadataKey;
	}

	set scopedMetadataKey(value) {
		this._scopedMetadataKey = value;
	}

	get targetNamespaceId() {
		return this._targetNamespaceId;
	}

	set targetNamespaceId(value) {
		this._targetNamespaceId = value;
	}

	get valueSizeDelta() {
		return this._valueSizeDelta;
	}

	set valueSizeDelta(value) {
		this._valueSizeDelta = value;
	}

	get value() {
		return this._value;
	}

	set value(value) {
		this._value = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetNamespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(targetNamespaceId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._targetAddress.serialize());
		buffer.write(converter.intToBytes(this._scopedMetadataKey, 8, false));
		buffer.write(this._targetNamespaceId.serialize());
		buffer.write(converter.intToBytes(this._valueSizeDelta, 2, true));
		buffer.write(converter.intToBytes(this._value.length, 2, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += `scopedMetadataKey: ${'0x'.concat(this._scopedMetadataKey.toString(16))}, `;
		result += `targetNamespaceId: ${this._targetNamespaceId.toString()}, `;
		result += `valueSizeDelta: ${'0x'.concat(this._valueSizeDelta.toString(16))}, `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
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

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicFlags(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		const values = [
			0, 1, 2, 4, 8
		];
		const keys = [
			'NONE', 'SUPPLY_MUTABLE', 'TRANSFERABLE', 'RESTRICTABLE', 'REVOKABLE'
		];

		if (0 === this.value) {
			const index = values.indexOf(this.value);
			return `MosaicFlags.${keys[index]}`;
		}

		const positions = values.map(flag => (this.value & flag)).filter(n => n).map(n => values.indexOf(n));
		return positions.map(n => `MosaicFlags.${keys[n]}`).join('|');
	}
}

class MosaicSupplyChangeAction {
	static DECREASE = new MosaicSupplyChangeAction(0);

	static INCREASE = new MosaicSupplyChangeAction(1);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			0, 1
		];
		const keys = [
			'DECREASE', 'INCREASE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return MosaicSupplyChangeAction[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `MosaicSupplyChangeAction.${MosaicSupplyChangeAction.valueToKey(this.value)}`;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get id() {
		return this._id;
	}

	set id(value) {
		this._id = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get nonce() {
		return this._nonce;
	}

	set nonce(value) {
		this._nonce = value;
	}

	get flags() {
		return this._flags;
	}

	set flags(value) {
		this._flags = value;
	}

	get divisibility() {
		return this._divisibility;
	}

	set divisibility(value) {
		this._divisibility = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const id = MosaicId.deserialize(view.buffer);
		view.shiftRight(id.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shiftRight(duration.size);
		const nonce = MosaicNonce.deserialize(view.buffer);
		view.shiftRight(nonce.size);
		const flags = MosaicFlags.deserialize(view.buffer);
		view.shiftRight(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._id.serialize());
		buffer.write(this._duration.serialize());
		buffer.write(this._nonce.serialize());
		buffer.write(this._flags.serialize());
		buffer.write(converter.intToBytes(this._divisibility, 1, false));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `id: ${this._id.toString()}, `;
		result += `duration: ${this._duration.toString()}, `;
		result += `nonce: ${this._nonce.toString()}, `;
		result += `flags: ${this._flags.toString()}, `;
		result += `divisibility: ${'0x'.concat(this._divisibility.toString(16))}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get id() {
		return this._id;
	}

	set id(value) {
		this._id = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get nonce() {
		return this._nonce;
	}

	set nonce(value) {
		this._nonce = value;
	}

	get flags() {
		return this._flags;
	}

	set flags(value) {
		this._flags = value;
	}

	get divisibility() {
		return this._divisibility;
	}

	set divisibility(value) {
		this._divisibility = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const id = MosaicId.deserialize(view.buffer);
		view.shiftRight(id.size);
		const duration = BlockDuration.deserialize(view.buffer);
		view.shiftRight(duration.size);
		const nonce = MosaicNonce.deserialize(view.buffer);
		view.shiftRight(nonce.size);
		const flags = MosaicFlags.deserialize(view.buffer);
		view.shiftRight(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._id.serialize());
		buffer.write(this._duration.serialize());
		buffer.write(this._nonce.serialize());
		buffer.write(this._flags.serialize());
		buffer.write(converter.intToBytes(this._divisibility, 1, false));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `id: ${this._id.toString()}, `;
		result += `duration: ${this._duration.toString()}, `;
		result += `nonce: ${this._nonce.toString()}, `;
		result += `flags: ${this._flags.toString()}, `;
		result += `divisibility: ${'0x'.concat(this._divisibility.toString(16))}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get delta() {
		return this._delta;
	}

	set delta(value) {
		this._delta = value;
	}

	get action() {
		return this._action;
	}

	set action(value) {
		this._action = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const delta = Amount.deserialize(view.buffer);
		view.shiftRight(delta.size);
		const action = MosaicSupplyChangeAction.deserialize(view.buffer);
		view.shiftRight(action.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._delta.serialize());
		buffer.write(this._action.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `delta: ${this._delta.toString()}, `;
		result += `action: ${this._action.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get delta() {
		return this._delta;
	}

	set delta(value) {
		this._delta = value;
	}

	get action() {
		return this._action;
	}

	set action(value) {
		this._action = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const delta = Amount.deserialize(view.buffer);
		view.shiftRight(delta.size);
		const action = MosaicSupplyChangeAction.deserialize(view.buffer);
		view.shiftRight(action.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._delta.serialize());
		buffer.write(this._action.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `delta: ${this._delta.toString()}, `;
		result += `action: ${this._action.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get sourceAddress() {
		return this._sourceAddress;
	}

	set sourceAddress(value) {
		this._sourceAddress = value;
	}

	get mosaic() {
		return this._mosaic;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const sourceAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(sourceAddress.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._sourceAddress.serialize());
		buffer.write(this._mosaic.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `sourceAddress: ${this._sourceAddress.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get sourceAddress() {
		return this._sourceAddress;
	}

	set sourceAddress(value) {
		this._sourceAddress = value;
	}

	get mosaic() {
		return this._mosaic;
	}

	set mosaic(value) {
		this._mosaic = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		size += this.sourceAddress.size;
		size += this.mosaic.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const sourceAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(sourceAddress.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._sourceAddress.serialize());
		buffer.write(this._mosaic.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `sourceAddress: ${this._sourceAddress.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._multisigAccountModificationTransactionBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get minRemovalDelta() {
		return this._minRemovalDelta;
	}

	set minRemovalDelta(value) {
		this._minRemovalDelta = value;
	}

	get minApprovalDelta() {
		return this._minApprovalDelta;
	}

	set minApprovalDelta(value) {
		this._minApprovalDelta = value;
	}

	get addressAdditions() {
		return this._addressAdditions;
	}

	set addressAdditions(value) {
		this._addressAdditions = value;
	}

	get addressDeletions() {
		return this._addressDeletions;
	}

	set addressDeletions(value) {
		this._addressDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.addressAdditions);
		size += arrayHelpers.size(this.addressDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const minRemovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shiftRight(1);
		const minApprovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shiftRight(1);
		const addressAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const addressDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const multisigAccountModificationTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== multisigAccountModificationTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${multisigAccountModificationTransactionBodyReserved_1})`);
		const addressAdditions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, addressAdditionsCount);
		view.shiftRight(arrayHelpers.size(addressAdditions));
		const addressDeletions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, addressDeletionsCount);
		view.shiftRight(arrayHelpers.size(addressDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._minRemovalDelta, 1, true));
		buffer.write(converter.intToBytes(this._minApprovalDelta, 1, true));
		buffer.write(converter.intToBytes(this._addressAdditions.length, 1, false)); // bound: address_additions_count
		buffer.write(converter.intToBytes(this._addressDeletions.length, 1, false)); // bound: address_deletions_count
		buffer.write(converter.intToBytes(this._multisigAccountModificationTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._addressAdditions);
		arrayHelpers.writeArray(buffer, this._addressDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `minRemovalDelta: ${'0x'.concat(this._minRemovalDelta.toString(16))}, `;
		result += `minApprovalDelta: ${'0x'.concat(this._minApprovalDelta.toString(16))}, `;
		result += `addressAdditions: [${this._addressAdditions.map(e => e.toString()).join(',')}], `;
		result += `addressDeletions: [${this._addressDeletions.map(e => e.toString()).join(',')}], `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._multisigAccountModificationTransactionBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get minRemovalDelta() {
		return this._minRemovalDelta;
	}

	set minRemovalDelta(value) {
		this._minRemovalDelta = value;
	}

	get minApprovalDelta() {
		return this._minApprovalDelta;
	}

	set minApprovalDelta(value) {
		this._minApprovalDelta = value;
	}

	get addressAdditions() {
		return this._addressAdditions;
	}

	set addressAdditions(value) {
		this._addressAdditions = value;
	}

	get addressDeletions() {
		return this._addressDeletions;
	}

	set addressDeletions(value) {
		this._addressDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.addressAdditions);
		size += arrayHelpers.size(this.addressDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const minRemovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shiftRight(1);
		const minApprovalDelta = converter.bytesToInt(view.buffer, 1, true);
		view.shiftRight(1);
		const addressAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const addressDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const multisigAccountModificationTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== multisigAccountModificationTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${multisigAccountModificationTransactionBodyReserved_1})`);
		const addressAdditions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, addressAdditionsCount);
		view.shiftRight(arrayHelpers.size(addressAdditions));
		const addressDeletions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, addressDeletionsCount);
		view.shiftRight(arrayHelpers.size(addressDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._minRemovalDelta, 1, true));
		buffer.write(converter.intToBytes(this._minApprovalDelta, 1, true));
		buffer.write(converter.intToBytes(this._addressAdditions.length, 1, false)); // bound: address_additions_count
		buffer.write(converter.intToBytes(this._addressDeletions.length, 1, false)); // bound: address_deletions_count
		buffer.write(converter.intToBytes(this._multisigAccountModificationTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._addressAdditions);
		arrayHelpers.writeArray(buffer, this._addressDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `minRemovalDelta: ${'0x'.concat(this._minRemovalDelta.toString(16))}, `;
		result += `minApprovalDelta: ${'0x'.concat(this._minApprovalDelta.toString(16))}, `;
		result += `addressAdditions: [${this._addressAdditions.map(e => e.toString()).join(',')}], `;
		result += `addressDeletions: [${this._addressDeletions.map(e => e.toString()).join(',')}], `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	get address() {
		return this._address;
	}

	set address(value) {
		this._address = value;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(namespaceId.size);
		const address = Address.deserialize(view.buffer);
		view.shiftRight(address.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shiftRight(aliasAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._namespaceId.serialize());
		buffer.write(this._address.serialize());
		buffer.write(this._aliasAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `namespaceId: ${this._namespaceId.toString()}, `;
		result += `address: ${this._address.toString()}, `;
		result += `aliasAction: ${this._aliasAction.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	get address() {
		return this._address;
	}

	set address(value) {
		this._address = value;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(namespaceId.size);
		const address = Address.deserialize(view.buffer);
		view.shiftRight(address.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shiftRight(aliasAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._namespaceId.serialize());
		buffer.write(this._address.serialize());
		buffer.write(this._aliasAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `namespaceId: ${this._namespaceId.toString()}, `;
		result += `address: ${this._address.toString()}, `;
		result += `aliasAction: ${this._aliasAction.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(namespaceId.size);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shiftRight(aliasAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._namespaceId.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._aliasAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `namespaceId: ${this._namespaceId.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `aliasAction: ${this._aliasAction.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get namespaceId() {
		return this._namespaceId;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get aliasAction() {
		return this._aliasAction;
	}

	set aliasAction(value) {
		this._aliasAction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(namespaceId.size);
		const mosaicId = MosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const aliasAction = AliasAction.deserialize(view.buffer);
		view.shiftRight(aliasAction.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._namespaceId.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._aliasAction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `namespaceId: ${this._namespaceId.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `aliasAction: ${this._aliasAction.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get parentId() {
		return this._parentId;
	}

	set parentId(value) {
		this._parentId = value;
	}

	get id() {
		return this._id;
	}

	set id(value) {
		this._id = value;
	}

	get registrationType() {
		return this._registrationType;
	}

	set registrationType(value) {
		this._registrationType = value;
	}

	get name() {
		return this._name;
	}

	set name(value) {
		this._name = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		if (NamespaceRegistrationType.ROOT === this.registrationType)
			size += this.duration.size;

		if (NamespaceRegistrationType.CHILD === this.registrationType)
			size += this.parentId.size;

		size += this.id.size;
		size += this.registrationType.size;
		size += 1;
		size += this._name.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		// deserialize to temporary buffer for further processing
		const durationTemporary = BlockDuration.deserialize(view.buffer);
		const registration_type_condition = view.window(durationTemporary.size);
		view.shiftRight(durationTemporary.size); // skip temporary

		const id = NamespaceId.deserialize(view.buffer);
		view.shiftRight(id.size);
		const registrationType = NamespaceRegistrationType.deserialize(view.buffer);
		view.shiftRight(registrationType.size);
		let duration;
		if (NamespaceRegistrationType.ROOT === registrationType)
			duration = BlockDuration.deserialize(registration_type_condition);

		let parentId;
		if (NamespaceRegistrationType.CHILD === registrationType)
			parentId = NamespaceId.deserialize(registration_type_condition);

		const nameSize = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		if (NamespaceRegistrationType.ROOT === this.registrationType)
			buffer.write(this._duration.serialize());

		if (NamespaceRegistrationType.CHILD === this.registrationType)
			buffer.write(this._parentId.serialize());

		buffer.write(this._id.serialize());
		buffer.write(this._registrationType.serialize());
		buffer.write(converter.intToBytes(this._name.length, 1, false)); // bound: name_size
		buffer.write(this._name);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		if (NamespaceRegistrationType.ROOT === this.registrationType)
			result += `duration: ${this._duration.toString()}, `;

		if (NamespaceRegistrationType.CHILD === this.registrationType)
			result += `parentId: ${this._parentId.toString()}, `;

		result += `id: ${this._id.toString()}, `;
		result += `registrationType: ${this._registrationType.toString()}, `;
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get duration() {
		return this._duration;
	}

	set duration(value) {
		this._duration = value;
	}

	get parentId() {
		return this._parentId;
	}

	set parentId(value) {
		this._parentId = value;
	}

	get id() {
		return this._id;
	}

	set id(value) {
		this._id = value;
	}

	get registrationType() {
		return this._registrationType;
	}

	set registrationType(value) {
		this._registrationType = value;
	}

	get name() {
		return this._name;
	}

	set name(value) {
		this._name = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += 1;
		size += this.network.size;
		size += this.type.size;
		if (NamespaceRegistrationType.ROOT === this.registrationType)
			size += this.duration.size;

		if (NamespaceRegistrationType.CHILD === this.registrationType)
			size += this.parentId.size;

		size += this.id.size;
		size += this.registrationType.size;
		size += 1;
		size += this._name.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		// deserialize to temporary buffer for further processing
		const durationTemporary = BlockDuration.deserialize(view.buffer);
		const registration_type_condition = view.window(durationTemporary.size);
		view.shiftRight(durationTemporary.size); // skip temporary

		const id = NamespaceId.deserialize(view.buffer);
		view.shiftRight(id.size);
		const registrationType = NamespaceRegistrationType.deserialize(view.buffer);
		view.shiftRight(registrationType.size);
		let duration;
		if (NamespaceRegistrationType.ROOT === registrationType)
			duration = BlockDuration.deserialize(registration_type_condition);

		let parentId;
		if (NamespaceRegistrationType.CHILD === registrationType)
			parentId = NamespaceId.deserialize(registration_type_condition);

		const nameSize = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		if (NamespaceRegistrationType.ROOT === this.registrationType)
			buffer.write(this._duration.serialize());

		if (NamespaceRegistrationType.CHILD === this.registrationType)
			buffer.write(this._parentId.serialize());

		buffer.write(this._id.serialize());
		buffer.write(this._registrationType.serialize());
		buffer.write(converter.intToBytes(this._name.length, 1, false)); // bound: name_size
		buffer.write(this._name);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		if (NamespaceRegistrationType.ROOT === this.registrationType)
			result += `duration: ${this._duration.toString()}, `;

		if (NamespaceRegistrationType.CHILD === this.registrationType)
			result += `parentId: ${this._parentId.toString()}, `;

		result += `id: ${this._id.toString()}, `;
		result += `registrationType: ${this._registrationType.toString()}, `;
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
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

	get size() { // eslint-disable-line class-methods-use-this
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new AccountRestrictionFlags(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}

	toString() {
		const values = [
			1, 2, 4, 16384, 32768
		];
		const keys = [
			'ADDRESS', 'MOSAIC_ID', 'TRANSACTION_TYPE', 'OUTGOING', 'BLOCK'
		];

		if (0 === this.value) {
			const index = values.indexOf(this.value);
			return `AccountRestrictionFlags.${keys[index]}`;
		}

		const positions = values.map(flag => (this.value & flag)).filter(n => n).map(n => values.indexOf(n));
		return positions.map(n => `AccountRestrictionFlags.${keys[n]}`).join('|');
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.restrictionAdditions);
		size += arrayHelpers.size(this.restrictionDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shiftRight(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== accountRestrictionTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, restrictionAdditionsCount);
		view.shiftRight(arrayHelpers.size(restrictionAdditions));
		const restrictionDeletions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, restrictionDeletionsCount);
		view.shiftRight(arrayHelpers.size(restrictionDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._restrictionFlags.serialize());
		buffer.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); // bound: restriction_additions_count
		buffer.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); // bound: restriction_deletions_count
		buffer.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._restrictionAdditions);
		arrayHelpers.writeArray(buffer, this._restrictionDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}, `;
		result += `restrictionAdditions: [${this._restrictionAdditions.map(e => e.toString()).join(',')}], `;
		result += `restrictionDeletions: [${this._restrictionDeletions.map(e => e.toString()).join(',')}], `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.restrictionAdditions);
		size += arrayHelpers.size(this.restrictionDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shiftRight(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== accountRestrictionTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, restrictionAdditionsCount);
		view.shiftRight(arrayHelpers.size(restrictionAdditions));
		const restrictionDeletions = arrayHelpers.readArrayCount(view.buffer, UnresolvedAddress, restrictionDeletionsCount);
		view.shiftRight(arrayHelpers.size(restrictionDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._restrictionFlags.serialize());
		buffer.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); // bound: restriction_additions_count
		buffer.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); // bound: restriction_deletions_count
		buffer.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._restrictionAdditions);
		arrayHelpers.writeArray(buffer, this._restrictionDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}, `;
		result += `restrictionAdditions: [${this._restrictionAdditions.map(e => e.toString()).join(',')}], `;
		result += `restrictionDeletions: [${this._restrictionDeletions.map(e => e.toString()).join(',')}], `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.restrictionAdditions);
		size += arrayHelpers.size(this.restrictionDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shiftRight(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== accountRestrictionTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaicId, restrictionAdditionsCount);
		view.shiftRight(arrayHelpers.size(restrictionAdditions));
		const restrictionDeletions = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaicId, restrictionDeletionsCount);
		view.shiftRight(arrayHelpers.size(restrictionDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._restrictionFlags.serialize());
		buffer.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); // bound: restriction_additions_count
		buffer.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); // bound: restriction_deletions_count
		buffer.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._restrictionAdditions);
		arrayHelpers.writeArray(buffer, this._restrictionDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}, `;
		result += `restrictionAdditions: [${this._restrictionAdditions.map(e => e.toString()).join(',')}], `;
		result += `restrictionDeletions: [${this._restrictionDeletions.map(e => e.toString()).join(',')}], `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.restrictionAdditions);
		size += arrayHelpers.size(this.restrictionDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shiftRight(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== accountRestrictionTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaicId, restrictionAdditionsCount);
		view.shiftRight(arrayHelpers.size(restrictionAdditions));
		const restrictionDeletions = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaicId, restrictionDeletionsCount);
		view.shiftRight(arrayHelpers.size(restrictionDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._restrictionFlags.serialize());
		buffer.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); // bound: restriction_additions_count
		buffer.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); // bound: restriction_deletions_count
		buffer.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._restrictionAdditions);
		arrayHelpers.writeArray(buffer, this._restrictionDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}, `;
		result += `restrictionAdditions: [${this._restrictionAdditions.map(e => e.toString()).join(',')}], `;
		result += `restrictionDeletions: [${this._restrictionDeletions.map(e => e.toString()).join(',')}], `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.restrictionAdditions);
		size += arrayHelpers.size(this.restrictionDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shiftRight(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== accountRestrictionTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.readArrayCount(view.buffer, TransactionType, restrictionAdditionsCount);
		view.shiftRight(arrayHelpers.size(restrictionAdditions));
		const restrictionDeletions = arrayHelpers.readArrayCount(view.buffer, TransactionType, restrictionDeletionsCount);
		view.shiftRight(arrayHelpers.size(restrictionDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._restrictionFlags.serialize());
		buffer.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); // bound: restriction_additions_count
		buffer.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); // bound: restriction_deletions_count
		buffer.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._restrictionAdditions);
		arrayHelpers.writeArray(buffer, this._restrictionDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}, `;
		result += `restrictionAdditions: [${this._restrictionAdditions.map(e => e.toString()).join(',')}], `;
		result += `restrictionDeletions: [${this._restrictionDeletions.map(e => e.toString()).join(',')}], `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get restrictionFlags() {
		return this._restrictionFlags;
	}

	set restrictionFlags(value) {
		this._restrictionFlags = value;
	}

	get restrictionAdditions() {
		return this._restrictionAdditions;
	}

	set restrictionAdditions(value) {
		this._restrictionAdditions = value;
	}

	get restrictionDeletions() {
		return this._restrictionDeletions;
	}

	set restrictionDeletions(value) {
		this._restrictionDeletions = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.restrictionAdditions);
		size += arrayHelpers.size(this.restrictionDeletions);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserialize(view.buffer);
		view.shiftRight(restrictionFlags.size);
		const restrictionAdditionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const restrictionDeletionsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const accountRestrictionTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== accountRestrictionTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${accountRestrictionTransactionBodyReserved_1})`);
		const restrictionAdditions = arrayHelpers.readArrayCount(view.buffer, TransactionType, restrictionAdditionsCount);
		view.shiftRight(arrayHelpers.size(restrictionAdditions));
		const restrictionDeletions = arrayHelpers.readArrayCount(view.buffer, TransactionType, restrictionDeletionsCount);
		view.shiftRight(arrayHelpers.size(restrictionDeletions));

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._restrictionFlags.serialize());
		buffer.write(converter.intToBytes(this._restrictionAdditions.length, 1, false)); // bound: restriction_additions_count
		buffer.write(converter.intToBytes(this._restrictionDeletions.length, 1, false)); // bound: restriction_deletions_count
		buffer.write(converter.intToBytes(this._accountRestrictionTransactionBodyReserved_1, 4, false));
		arrayHelpers.writeArray(buffer, this._restrictionAdditions);
		arrayHelpers.writeArray(buffer, this._restrictionDeletions);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `restrictionFlags: ${this._restrictionFlags.toString()}, `;
		result += `restrictionAdditions: [${this._restrictionAdditions.map(e => e.toString()).join(',')}], `;
		result += `restrictionDeletions: [${this._restrictionDeletions.map(e => e.toString()).join(',')}], `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `restrictionKey: ${'0x'.concat(this._restrictionKey.toString(16))}, `;
		result += `previousRestrictionValue: ${'0x'.concat(this._previousRestrictionValue.toString(16))}, `;
		result += `newRestrictionValue: ${'0x'.concat(this._newRestrictionValue.toString(16))}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `restrictionKey: ${'0x'.concat(this._restrictionKey.toString(16))}, `;
		result += `previousRestrictionValue: ${'0x'.concat(this._previousRestrictionValue.toString(16))}, `;
		result += `newRestrictionValue: ${'0x'.concat(this._newRestrictionValue.toString(16))}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
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

	static valueToKey(value) {
		const values = [
			0, 1, 2, 3, 4, 5, 6
		];
		const keys = [
			'NONE', 'EQ', 'NE', 'LT', 'LE', 'GT', 'GE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return MosaicRestrictionType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 1;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 1, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 1, false);
	}

	toString() {
		return `MosaicRestrictionType.${MosaicRestrictionType.valueToKey(this.value)}`;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get referenceMosaicId() {
		return this._referenceMosaicId;
	}

	set referenceMosaicId(value) {
		this._referenceMosaicId = value;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	get previousRestrictionType() {
		return this._previousRestrictionType;
	}

	set previousRestrictionType(value) {
		this._previousRestrictionType = value;
	}

	get newRestrictionType() {
		return this._newRestrictionType;
	}

	set newRestrictionType(value) {
		this._newRestrictionType = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const referenceMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(referenceMosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shiftRight(previousRestrictionType.size);
		const newRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shiftRight(newRestrictionType.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._referenceMosaicId.serialize());
		buffer.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer.write(this._previousRestrictionType.serialize());
		buffer.write(this._newRestrictionType.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `referenceMosaicId: ${this._referenceMosaicId.toString()}, `;
		result += `restrictionKey: ${'0x'.concat(this._restrictionKey.toString(16))}, `;
		result += `previousRestrictionValue: ${'0x'.concat(this._previousRestrictionValue.toString(16))}, `;
		result += `newRestrictionValue: ${'0x'.concat(this._newRestrictionValue.toString(16))}, `;
		result += `previousRestrictionType: ${this._previousRestrictionType.toString()}, `;
		result += `newRestrictionType: ${this._newRestrictionType.toString()}, `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get referenceMosaicId() {
		return this._referenceMosaicId;
	}

	set referenceMosaicId(value) {
		this._referenceMosaicId = value;
	}

	get restrictionKey() {
		return this._restrictionKey;
	}

	set restrictionKey(value) {
		this._restrictionKey = value;
	}

	get previousRestrictionValue() {
		return this._previousRestrictionValue;
	}

	set previousRestrictionValue(value) {
		this._previousRestrictionValue = value;
	}

	get newRestrictionValue() {
		return this._newRestrictionValue;
	}

	set newRestrictionValue(value) {
		this._newRestrictionValue = value;
	}

	get previousRestrictionType() {
		return this._previousRestrictionType;
	}

	set previousRestrictionType(value) {
		this._previousRestrictionType = value;
	}

	get newRestrictionType() {
		return this._newRestrictionType;
	}

	set newRestrictionType(value) {
		this._newRestrictionType = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(mosaicId.size);
		const referenceMosaicId = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(referenceMosaicId.size);
		const restrictionKey = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shiftRight(previousRestrictionType.size);
		const newRestrictionType = MosaicRestrictionType.deserialize(view.buffer);
		view.shiftRight(newRestrictionType.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._referenceMosaicId.serialize());
		buffer.write(converter.intToBytes(this._restrictionKey, 8, false));
		buffer.write(converter.intToBytes(this._previousRestrictionValue, 8, false));
		buffer.write(converter.intToBytes(this._newRestrictionValue, 8, false));
		buffer.write(this._previousRestrictionType.serialize());
		buffer.write(this._newRestrictionType.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `referenceMosaicId: ${this._referenceMosaicId.toString()}, `;
		result += `restrictionKey: ${'0x'.concat(this._restrictionKey.toString(16))}, `;
		result += `previousRestrictionValue: ${'0x'.concat(this._previousRestrictionValue.toString(16))}, `;
		result += `newRestrictionValue: ${'0x'.concat(this._newRestrictionValue.toString(16))}, `;
		result += `previousRestrictionType: ${this._previousRestrictionType.toString()}, `;
		result += `newRestrictionType: ${this._newRestrictionType.toString()}, `;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._transferTransactionBodyReserved_1 = 0; // reserved field
		this._transferTransactionBodyReserved_2 = 0; // reserved field
	}

	get signature() {
		return this._signature;
	}

	set signature(value) {
		this._signature = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get deadline() {
		return this._deadline;
	}

	set deadline(value) {
		this._deadline = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get mosaics() {
		return this._mosaics;
	}

	set mosaics(value) {
		this._mosaics = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.mosaics);
		size += this._message.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const verifiableEntityHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== verifiableEntityHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${verifiableEntityHeaderReserved_1})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const messageSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const mosaicsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const transferTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		if (0 !== transferTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_1})`);
		const transferTransactionBodyReserved_2 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== transferTransactionBodyReserved_2)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_2})`);
		const mosaics = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaic, mosaicsCount, e => e.mosaicId.value);
		view.shiftRight(arrayHelpers.size(mosaics));
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shiftRight(messageSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._recipientAddress.serialize());
		buffer.write(converter.intToBytes(this._message.length, 2, false)); // bound: message_size
		buffer.write(converter.intToBytes(this._mosaics.length, 1, false)); // bound: mosaics_count
		buffer.write(converter.intToBytes(this._transferTransactionBodyReserved_1, 1, false));
		buffer.write(converter.intToBytes(this._transferTransactionBodyReserved_2, 4, false));
		arrayHelpers.writeArray(buffer, this._mosaics, e => e.mosaicId.value);
		buffer.write(this._message);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `mosaics: [${this._mosaics.map(e => e.toString()).join(',')}], `;
		result += `message: hex(${converter.uint8ToHex(this._message)}), `;
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
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._transferTransactionBodyReserved_1 = 0; // reserved field
		this._transferTransactionBodyReserved_2 = 0; // reserved field
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get network() {
		return this._network;
	}

	set network(value) {
		this._network = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get mosaics() {
		return this._mosaics;
	}

	set mosaics(value) {
		this._mosaics = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
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
		size += arrayHelpers.size(this.mosaics);
		size += this._message.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const embeddedTransactionHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== embeddedTransactionHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${embeddedTransactionHeaderReserved_1})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const entityBodyReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const version = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const messageSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const mosaicsCount = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const transferTransactionBodyReserved_1 = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		if (0 !== transferTransactionBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_1})`);
		const transferTransactionBodyReserved_2 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== transferTransactionBodyReserved_2)
			throw RangeError(`Invalid value of reserved field (${transferTransactionBodyReserved_2})`);
		const mosaics = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaic, mosaicsCount, e => e.mosaicId.value);
		view.shiftRight(arrayHelpers.size(mosaics));
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shiftRight(messageSize);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._embeddedTransactionHeaderReserved_1, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._recipientAddress.serialize());
		buffer.write(converter.intToBytes(this._message.length, 2, false)); // bound: message_size
		buffer.write(converter.intToBytes(this._mosaics.length, 1, false)); // bound: mosaics_count
		buffer.write(converter.intToBytes(this._transferTransactionBodyReserved_1, 1, false));
		buffer.write(converter.intToBytes(this._transferTransactionBodyReserved_2, 4, false));
		arrayHelpers.writeArray(buffer, this._mosaics, e => e.mosaicId.value);
		buffer.write(this._message);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `mosaics: [${this._mosaics.map(e => e.toString()).join(',')}], `;
		result += `message: hex(${converter.uint8ToHex(this._message)}), `;
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
			account_key_link_transaction: AccountKeyLinkTransaction,
			node_key_link_transaction: NodeKeyLinkTransaction,
			aggregate_complete_transaction: AggregateCompleteTransaction,
			aggregate_bonded_transaction: AggregateBondedTransaction,
			voting_key_link_transaction: VotingKeyLinkTransaction,
			vrf_key_link_transaction: VrfKeyLinkTransaction,
			hash_lock_transaction: HashLockTransaction,
			secret_lock_transaction: SecretLockTransaction,
			secret_proof_transaction: SecretProofTransaction,
			account_metadata_transaction: AccountMetadataTransaction,
			mosaic_metadata_transaction: MosaicMetadataTransaction,
			namespace_metadata_transaction: NamespaceMetadataTransaction,
			mosaic_definition_transaction: MosaicDefinitionTransaction,
			mosaic_supply_change_transaction: MosaicSupplyChangeTransaction,
			mosaic_supply_revocation_transaction: MosaicSupplyRevocationTransaction,
			multisig_account_modification_transaction: MultisigAccountModificationTransaction,
			address_alias_transaction: AddressAliasTransaction,
			mosaic_alias_transaction: MosaicAliasTransaction,
			namespace_registration_transaction: NamespaceRegistrationTransaction,
			account_address_restriction_transaction: AccountAddressRestrictionTransaction,
			account_mosaic_restriction_transaction: AccountMosaicRestrictionTransaction,
			account_operation_restriction_transaction: AccountOperationRestrictionTransaction,
			mosaic_address_restriction_transaction: MosaicAddressRestrictionTransaction,
			mosaic_global_restriction_transaction: MosaicGlobalRestrictionTransaction,
			transfer_transaction: TransferTransaction
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError('unknown Transaction type');

		return new mapping[entityName]();
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
			account_key_link_transaction: EmbeddedAccountKeyLinkTransaction,
			node_key_link_transaction: EmbeddedNodeKeyLinkTransaction,
			voting_key_link_transaction: EmbeddedVotingKeyLinkTransaction,
			vrf_key_link_transaction: EmbeddedVrfKeyLinkTransaction,
			hash_lock_transaction: EmbeddedHashLockTransaction,
			secret_lock_transaction: EmbeddedSecretLockTransaction,
			secret_proof_transaction: EmbeddedSecretProofTransaction,
			account_metadata_transaction: EmbeddedAccountMetadataTransaction,
			mosaic_metadata_transaction: EmbeddedMosaicMetadataTransaction,
			namespace_metadata_transaction: EmbeddedNamespaceMetadataTransaction,
			mosaic_definition_transaction: EmbeddedMosaicDefinitionTransaction,
			mosaic_supply_change_transaction: EmbeddedMosaicSupplyChangeTransaction,
			mosaic_supply_revocation_transaction: EmbeddedMosaicSupplyRevocationTransaction,
			multisig_account_modification_transaction: EmbeddedMultisigAccountModificationTransaction,
			address_alias_transaction: EmbeddedAddressAliasTransaction,
			mosaic_alias_transaction: EmbeddedMosaicAliasTransaction,
			namespace_registration_transaction: EmbeddedNamespaceRegistrationTransaction,
			account_address_restriction_transaction: EmbeddedAccountAddressRestrictionTransaction,
			account_mosaic_restriction_transaction: EmbeddedAccountMosaicRestrictionTransaction,
			account_operation_restriction_transaction: EmbeddedAccountOperationRestrictionTransaction,
			mosaic_address_restriction_transaction: EmbeddedMosaicAddressRestrictionTransaction,
			mosaic_global_restriction_transaction: EmbeddedMosaicGlobalRestrictionTransaction,
			transfer_transaction: EmbeddedTransferTransaction
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError('unknown EmbeddedTransaction type');

		return new mapping[entityName]();
	}
}

module.exports = {
	Amount, BlockDuration, BlockFeeMultiplier, Difficulty, FinalizationEpoch, FinalizationPoint, Height, Importance, ImportanceHeight,
	UnresolvedMosaicId, MosaicId, Timestamp, UnresolvedAddress, Address, Hash256, Hash512, PublicKey, VotingPublicKey, Signature, Mosaic,
	UnresolvedMosaic, LinkAction, NetworkType, TransactionType, Transaction, EmbeddedTransaction, AccountKeyLinkTransaction,
	EmbeddedAccountKeyLinkTransaction, NodeKeyLinkTransaction, EmbeddedNodeKeyLinkTransaction, Cosignature, DetachedCosignature,
	AggregateCompleteTransaction, AggregateBondedTransaction, VotingKeyLinkTransaction, EmbeddedVotingKeyLinkTransaction, VrfKeyLinkTransaction,
	EmbeddedVrfKeyLinkTransaction, HashLockTransaction, EmbeddedHashLockTransaction, LockHashAlgorithm, SecretLockTransaction,
	EmbeddedSecretLockTransaction, SecretProofTransaction, EmbeddedSecretProofTransaction, AccountMetadataTransaction,
	EmbeddedAccountMetadataTransaction, MosaicMetadataTransaction, EmbeddedMosaicMetadataTransaction, NamespaceId, NamespaceRegistrationType,
	AliasAction, NamespaceMetadataTransaction, EmbeddedNamespaceMetadataTransaction, MosaicNonce, MosaicFlags, MosaicSupplyChangeAction,
	MosaicDefinitionTransaction, EmbeddedMosaicDefinitionTransaction, MosaicSupplyChangeTransaction, EmbeddedMosaicSupplyChangeTransaction,
	MosaicSupplyRevocationTransaction, EmbeddedMosaicSupplyRevocationTransaction, MultisigAccountModificationTransaction,
	EmbeddedMultisigAccountModificationTransaction, AddressAliasTransaction, EmbeddedAddressAliasTransaction, MosaicAliasTransaction,
	EmbeddedMosaicAliasTransaction, NamespaceRegistrationTransaction, EmbeddedNamespaceRegistrationTransaction, AccountRestrictionFlags,
	AccountAddressRestrictionTransaction, EmbeddedAccountAddressRestrictionTransaction, AccountMosaicRestrictionTransaction,
	EmbeddedAccountMosaicRestrictionTransaction, AccountOperationRestrictionTransaction, EmbeddedAccountOperationRestrictionTransaction,
	MosaicAddressRestrictionTransaction, EmbeddedMosaicAddressRestrictionTransaction, MosaicRestrictionKey, MosaicRestrictionType,
	MosaicGlobalRestrictionTransaction, EmbeddedMosaicGlobalRestrictionTransaction, TransferTransaction, EmbeddedTransferTransaction,
	TransactionFactory, EmbeddedTransactionFactory
};
