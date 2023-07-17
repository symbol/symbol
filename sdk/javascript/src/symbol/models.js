/* eslint-disable max-len, object-property-newline, no-underscore-dangle, no-use-before-define */

import BaseValue from '../BaseValue.js';
import ByteArray from '../ByteArray.js';
import BufferView from '../utils/BufferView.js';
import Writer from '../utils/Writer.js';
import * as arrayHelpers from '../utils/arrayHelpers.js';
import * as converter from '../utils/converter.js';

export class Amount extends BaseValue {
	static SIZE = 8;

	constructor(amount = 0n) {
		super(Amount.SIZE, amount);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Amount(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new Amount(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class BlockDuration extends BaseValue {
	static SIZE = 8;

	constructor(blockDuration = 0n) {
		super(BlockDuration.SIZE, blockDuration);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new BlockDuration(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new BlockDuration(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class BlockFeeMultiplier extends BaseValue {
	static SIZE = 4;

	constructor(blockFeeMultiplier = 0) {
		super(BlockFeeMultiplier.SIZE, blockFeeMultiplier);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new BlockFeeMultiplier(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new BlockFeeMultiplier(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

export class Difficulty extends BaseValue {
	static SIZE = 8;

	constructor(difficulty = 0n) {
		super(Difficulty.SIZE, difficulty);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Difficulty(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new Difficulty(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class FinalizationEpoch extends BaseValue {
	static SIZE = 4;

	constructor(finalizationEpoch = 0) {
		super(FinalizationEpoch.SIZE, finalizationEpoch);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new FinalizationEpoch(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new FinalizationEpoch(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

export class FinalizationPoint extends BaseValue {
	static SIZE = 4;

	constructor(finalizationPoint = 0) {
		super(FinalizationPoint.SIZE, finalizationPoint);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new FinalizationPoint(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new FinalizationPoint(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

export class Height extends BaseValue {
	static SIZE = 8;

	constructor(height = 0n) {
		super(Height.SIZE, height);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Height(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new Height(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class Importance extends BaseValue {
	static SIZE = 8;

	constructor(importance = 0n) {
		super(Importance.SIZE, importance);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Importance(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new Importance(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class ImportanceHeight extends BaseValue {
	static SIZE = 8;

	constructor(importanceHeight = 0n) {
		super(ImportanceHeight.SIZE, importanceHeight);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new ImportanceHeight(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new ImportanceHeight(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class UnresolvedMosaicId extends BaseValue {
	static SIZE = 8;

	constructor(unresolvedMosaicId = 0n) {
		super(UnresolvedMosaicId.SIZE, unresolvedMosaicId);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new UnresolvedMosaicId(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new UnresolvedMosaicId(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class MosaicId extends BaseValue {
	static SIZE = 8;

	constructor(mosaicId = 0n) {
		super(MosaicId.SIZE, mosaicId);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicId(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new MosaicId(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class Timestamp extends BaseValue {
	static SIZE = 8;

	constructor(timestamp = 0n) {
		super(Timestamp.SIZE, timestamp);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Timestamp(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new Timestamp(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class UnresolvedAddress extends ByteArray {
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

export class Address extends ByteArray {
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

export class Hash256 extends ByteArray {
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

export class Hash512 extends ByteArray {
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

export class PublicKey extends ByteArray {
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

export class VotingPublicKey extends ByteArray {
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

export class Signature extends ByteArray {
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

export class Mosaic {
	static TYPE_HINTS = {
		mosaicId: 'pod:MosaicId',
		amount: 'pod:Amount'
	};

	constructor() {
		this._mosaicId = new MosaicId();
		this._amount = new Amount();
	}

	sort() { // eslint-disable-line class-methods-use-this
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

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const mosaicId = MosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const amount = Amount.deserializeAligned(view.buffer);
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

export class UnresolvedMosaic {
	static TYPE_HINTS = {
		mosaicId: 'pod:UnresolvedMosaicId',
		amount: 'pod:Amount'
	};

	constructor() {
		this._mosaicId = new UnresolvedMosaicId();
		this._amount = new Amount();
	}

	sort() { // eslint-disable-line class-methods-use-this
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

export class LinkAction {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class NetworkType {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class TransactionType {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 2, false));
	}

	static deserializeAligned(payload) {
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

export class Transaction {
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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

export class EmbeddedTransaction {
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
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

export class ProofGamma extends ByteArray {
	static SIZE = 32;

	constructor(proofGamma = new Uint8Array(32)) {
		super(ProofGamma.SIZE, proofGamma);
	}

	get size() { // eslint-disable-line class-methods-use-this
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

export class ProofVerificationHash extends ByteArray {
	static SIZE = 16;

	constructor(proofVerificationHash = new Uint8Array(16)) {
		super(ProofVerificationHash.SIZE, proofVerificationHash);
	}

	get size() { // eslint-disable-line class-methods-use-this
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

export class ProofScalar extends ByteArray {
	static SIZE = 32;

	constructor(proofScalar = new Uint8Array(32)) {
		super(ProofScalar.SIZE, proofScalar);
	}

	get size() { // eslint-disable-line class-methods-use-this
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

export class BlockType {
	static NEMESIS = new BlockType(32835);

	static NORMAL = new BlockType(33091);

	static IMPORTANCE = new BlockType(33347);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			32835, 33091, 33347
		];
		const keys = [
			'NEMESIS', 'NORMAL', 'IMPORTANCE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return BlockType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 2, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}

	toString() {
		return `BlockType.${BlockType.valueToKey(this.value)}`;
	}
}

export class VrfProof {
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

	sort() { // eslint-disable-line class-methods-use-this
	}

	get gamma() {
		return this._gamma;
	}

	set gamma(value) {
		this._gamma = value;
	}

	get verificationHash() {
		return this._verificationHash;
	}

	set verificationHash(value) {
		this._verificationHash = value;
	}

	get scalar() {
		return this._scalar;
	}

	set scalar(value) {
		this._scalar = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.gamma.size;
		size += this.verificationHash.size;
		size += this.scalar.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const gamma = ProofGamma.deserialize(view.buffer);
		view.shiftRight(gamma.size);
		const verificationHash = ProofVerificationHash.deserialize(view.buffer);
		view.shiftRight(verificationHash.size);
		const scalar = ProofScalar.deserialize(view.buffer);
		view.shiftRight(scalar.size);

		const instance = new VrfProof();
		instance._gamma = gamma;
		instance._verificationHash = verificationHash;
		instance._scalar = scalar;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._gamma.serialize());
		buffer.write(this._verificationHash.serialize());
		buffer.write(this._scalar.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `gamma: ${this._gamma.toString()}, `;
		result += `verificationHash: ${this._verificationHash.toString()}, `;
		result += `scalar: ${this._scalar.toString()}, `;
		result += ')';
		return result;
	}
}

export class Block {
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._generationHashProof.sort();
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

	get height() {
		return this._height;
	}

	set height(value) {
		this._height = value;
	}

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get difficulty() {
		return this._difficulty;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	get stateHash() {
		return this._stateHash;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = BlockType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const height = Height.deserializeAligned(view.buffer);
		view.shiftRight(height.size);
		const timestamp = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(timestamp.size);
		const difficulty = Difficulty.deserializeAligned(view.buffer);
		view.shiftRight(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shiftRight(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shiftRight(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shiftRight(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shiftRight(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserializeAligned(view.buffer);
		view.shiftRight(feeMultiplier.size);

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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._height.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(this._difficulty.serialize());
		buffer.write(this._generationHashProof.serialize());
		buffer.write(this._previousBlockHash.serialize());
		buffer.write(this._transactionsHash.serialize());
		buffer.write(this._receiptsHash.serialize());
		buffer.write(this._stateHash.serialize());
		buffer.write(this._beneficiaryAddress.serialize());
		buffer.write(this._feeMultiplier.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `height: ${this._height.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `difficulty: ${this._difficulty.toString()}, `;
		result += `generationHashProof: ${this._generationHashProof.toString()}, `;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}, `;
		result += `transactionsHash: ${this._transactionsHash.toString()}, `;
		result += `receiptsHash: ${this._receiptsHash.toString()}, `;
		result += `stateHash: ${this._stateHash.toString()}, `;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}, `;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}, `;
		result += ')';
		return result;
	}
}

export class NemesisBlockV1 {
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
		this._version = NemesisBlockV1.BLOCK_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NemesisBlockV1.BLOCK_TYPE;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._generationHashProof.sort();
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

	get height() {
		return this._height;
	}

	set height(value) {
		this._height = value;
	}

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get difficulty() {
		return this._difficulty;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	get stateHash() {
		return this._stateHash;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	get votingEligibleAccountsCount() {
		return this._votingEligibleAccountsCount;
	}

	set votingEligibleAccountsCount(value) {
		this._votingEligibleAccountsCount = value;
	}

	get harvestingEligibleAccountsCount() {
		return this._harvestingEligibleAccountsCount;
	}

	set harvestingEligibleAccountsCount(value) {
		this._harvestingEligibleAccountsCount = value;
	}

	get totalVotingBalance() {
		return this._totalVotingBalance;
	}

	set totalVotingBalance(value) {
		this._totalVotingBalance = value;
	}

	get previousImportanceBlockHash() {
		return this._previousImportanceBlockHash;
	}

	set previousImportanceBlockHash(value) {
		this._previousImportanceBlockHash = value;
	}

	get transactions() {
		return this._transactions;
	}

	set transactions(value) {
		this._transactions = value;
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
		size += arrayHelpers.size(this.transactions, 8, true);
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = BlockType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const height = Height.deserializeAligned(view.buffer);
		view.shiftRight(height.size);
		const timestamp = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(timestamp.size);
		const difficulty = Difficulty.deserializeAligned(view.buffer);
		view.shiftRight(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shiftRight(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shiftRight(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shiftRight(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shiftRight(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserializeAligned(view.buffer);
		view.shiftRight(feeMultiplier.size);
		const votingEligibleAccountsCount = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		const harvestingEligibleAccountsCount = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const totalVotingBalance = Amount.deserializeAligned(view.buffer);
		view.shiftRight(totalVotingBalance.size);
		const previousImportanceBlockHash = Hash256.deserialize(view.buffer);
		view.shiftRight(previousImportanceBlockHash.size);
		const transactions = arrayHelpers.readVariableSizeElements(view.buffer, TransactionFactory, 8, true);
		view.shiftRight(arrayHelpers.size(transactions, 8, true));

		const instance = new NemesisBlockV1();
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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._height.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(this._difficulty.serialize());
		buffer.write(this._generationHashProof.serialize());
		buffer.write(this._previousBlockHash.serialize());
		buffer.write(this._transactionsHash.serialize());
		buffer.write(this._receiptsHash.serialize());
		buffer.write(this._stateHash.serialize());
		buffer.write(this._beneficiaryAddress.serialize());
		buffer.write(this._feeMultiplier.serialize());
		buffer.write(converter.intToBytes(this._votingEligibleAccountsCount, 4, false));
		buffer.write(converter.intToBytes(this._harvestingEligibleAccountsCount, 8, false));
		buffer.write(this._totalVotingBalance.serialize());
		buffer.write(this._previousImportanceBlockHash.serialize());
		arrayHelpers.writeVariableSizeElements(buffer, this._transactions, 8, true);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `height: ${this._height.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `difficulty: ${this._difficulty.toString()}, `;
		result += `generationHashProof: ${this._generationHashProof.toString()}, `;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}, `;
		result += `transactionsHash: ${this._transactionsHash.toString()}, `;
		result += `receiptsHash: ${this._receiptsHash.toString()}, `;
		result += `stateHash: ${this._stateHash.toString()}, `;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}, `;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}, `;
		result += `votingEligibleAccountsCount: ${'0x'.concat(this._votingEligibleAccountsCount.toString(16))}, `;
		result += `harvestingEligibleAccountsCount: ${'0x'.concat(this._harvestingEligibleAccountsCount.toString(16))}, `;
		result += `totalVotingBalance: ${this._totalVotingBalance.toString()}, `;
		result += `previousImportanceBlockHash: ${this._previousImportanceBlockHash.toString()}, `;
		result += `transactions: [${this._transactions.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class NormalBlockV1 {
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
		this._version = NormalBlockV1.BLOCK_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NormalBlockV1.BLOCK_TYPE;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._blockHeaderReserved_1 = 0; // reserved field
	}

	sort() {
		this._generationHashProof.sort();
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

	get height() {
		return this._height;
	}

	set height(value) {
		this._height = value;
	}

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get difficulty() {
		return this._difficulty;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	get stateHash() {
		return this._stateHash;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	get transactions() {
		return this._transactions;
	}

	set transactions(value) {
		this._transactions = value;
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
		size += arrayHelpers.size(this.transactions, 8, true);
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = BlockType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const height = Height.deserializeAligned(view.buffer);
		view.shiftRight(height.size);
		const timestamp = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(timestamp.size);
		const difficulty = Difficulty.deserializeAligned(view.buffer);
		view.shiftRight(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shiftRight(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shiftRight(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shiftRight(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shiftRight(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserializeAligned(view.buffer);
		view.shiftRight(feeMultiplier.size);
		const blockHeaderReserved_1 = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		if (0 !== blockHeaderReserved_1)
			throw RangeError(`Invalid value of reserved field (${blockHeaderReserved_1})`);
		const transactions = arrayHelpers.readVariableSizeElements(view.buffer, TransactionFactory, 8, true);
		view.shiftRight(arrayHelpers.size(transactions, 8, true));

		const instance = new NormalBlockV1();
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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._height.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(this._difficulty.serialize());
		buffer.write(this._generationHashProof.serialize());
		buffer.write(this._previousBlockHash.serialize());
		buffer.write(this._transactionsHash.serialize());
		buffer.write(this._receiptsHash.serialize());
		buffer.write(this._stateHash.serialize());
		buffer.write(this._beneficiaryAddress.serialize());
		buffer.write(this._feeMultiplier.serialize());
		buffer.write(converter.intToBytes(this._blockHeaderReserved_1, 4, false));
		arrayHelpers.writeVariableSizeElements(buffer, this._transactions, 8, true);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `height: ${this._height.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `difficulty: ${this._difficulty.toString()}, `;
		result += `generationHashProof: ${this._generationHashProof.toString()}, `;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}, `;
		result += `transactionsHash: ${this._transactionsHash.toString()}, `;
		result += `receiptsHash: ${this._receiptsHash.toString()}, `;
		result += `stateHash: ${this._stateHash.toString()}, `;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}, `;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}, `;
		result += `transactions: [${this._transactions.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class ImportanceBlockV1 {
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
		this._version = ImportanceBlockV1.BLOCK_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = ImportanceBlockV1.BLOCK_TYPE;
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
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._generationHashProof.sort();
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

	get height() {
		return this._height;
	}

	set height(value) {
		this._height = value;
	}

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get difficulty() {
		return this._difficulty;
	}

	set difficulty(value) {
		this._difficulty = value;
	}

	get generationHashProof() {
		return this._generationHashProof;
	}

	set generationHashProof(value) {
		this._generationHashProof = value;
	}

	get previousBlockHash() {
		return this._previousBlockHash;
	}

	set previousBlockHash(value) {
		this._previousBlockHash = value;
	}

	get transactionsHash() {
		return this._transactionsHash;
	}

	set transactionsHash(value) {
		this._transactionsHash = value;
	}

	get receiptsHash() {
		return this._receiptsHash;
	}

	set receiptsHash(value) {
		this._receiptsHash = value;
	}

	get stateHash() {
		return this._stateHash;
	}

	set stateHash(value) {
		this._stateHash = value;
	}

	get beneficiaryAddress() {
		return this._beneficiaryAddress;
	}

	set beneficiaryAddress(value) {
		this._beneficiaryAddress = value;
	}

	get feeMultiplier() {
		return this._feeMultiplier;
	}

	set feeMultiplier(value) {
		this._feeMultiplier = value;
	}

	get votingEligibleAccountsCount() {
		return this._votingEligibleAccountsCount;
	}

	set votingEligibleAccountsCount(value) {
		this._votingEligibleAccountsCount = value;
	}

	get harvestingEligibleAccountsCount() {
		return this._harvestingEligibleAccountsCount;
	}

	set harvestingEligibleAccountsCount(value) {
		this._harvestingEligibleAccountsCount = value;
	}

	get totalVotingBalance() {
		return this._totalVotingBalance;
	}

	set totalVotingBalance(value) {
		this._totalVotingBalance = value;
	}

	get previousImportanceBlockHash() {
		return this._previousImportanceBlockHash;
	}

	set previousImportanceBlockHash(value) {
		this._previousImportanceBlockHash = value;
	}

	get transactions() {
		return this._transactions;
	}

	set transactions(value) {
		this._transactions = value;
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
		size += arrayHelpers.size(this.transactions, 8, true);
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = BlockType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const height = Height.deserializeAligned(view.buffer);
		view.shiftRight(height.size);
		const timestamp = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(timestamp.size);
		const difficulty = Difficulty.deserializeAligned(view.buffer);
		view.shiftRight(difficulty.size);
		const generationHashProof = VrfProof.deserialize(view.buffer);
		view.shiftRight(generationHashProof.size);
		const previousBlockHash = Hash256.deserialize(view.buffer);
		view.shiftRight(previousBlockHash.size);
		const transactionsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(transactionsHash.size);
		const receiptsHash = Hash256.deserialize(view.buffer);
		view.shiftRight(receiptsHash.size);
		const stateHash = Hash256.deserialize(view.buffer);
		view.shiftRight(stateHash.size);
		const beneficiaryAddress = Address.deserialize(view.buffer);
		view.shiftRight(beneficiaryAddress.size);
		const feeMultiplier = BlockFeeMultiplier.deserializeAligned(view.buffer);
		view.shiftRight(feeMultiplier.size);
		const votingEligibleAccountsCount = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		const harvestingEligibleAccountsCount = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const totalVotingBalance = Amount.deserializeAligned(view.buffer);
		view.shiftRight(totalVotingBalance.size);
		const previousImportanceBlockHash = Hash256.deserialize(view.buffer);
		view.shiftRight(previousImportanceBlockHash.size);
		const transactions = arrayHelpers.readVariableSizeElements(view.buffer, TransactionFactory, 8, true);
		view.shiftRight(arrayHelpers.size(transactions, 8, true));

		const instance = new ImportanceBlockV1();
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
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._verifiableEntityHeaderReserved_1, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 4, false));
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(this._network.serialize());
		buffer.write(this._type.serialize());
		buffer.write(this._height.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(this._difficulty.serialize());
		buffer.write(this._generationHashProof.serialize());
		buffer.write(this._previousBlockHash.serialize());
		buffer.write(this._transactionsHash.serialize());
		buffer.write(this._receiptsHash.serialize());
		buffer.write(this._stateHash.serialize());
		buffer.write(this._beneficiaryAddress.serialize());
		buffer.write(this._feeMultiplier.serialize());
		buffer.write(converter.intToBytes(this._votingEligibleAccountsCount, 4, false));
		buffer.write(converter.intToBytes(this._harvestingEligibleAccountsCount, 8, false));
		buffer.write(this._totalVotingBalance.serialize());
		buffer.write(this._previousImportanceBlockHash.serialize());
		arrayHelpers.writeVariableSizeElements(buffer, this._transactions, 8, true);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `signature: ${this._signature.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `type: ${this._type.toString()}, `;
		result += `height: ${this._height.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `difficulty: ${this._difficulty.toString()}, `;
		result += `generationHashProof: ${this._generationHashProof.toString()}, `;
		result += `previousBlockHash: ${this._previousBlockHash.toString()}, `;
		result += `transactionsHash: ${this._transactionsHash.toString()}, `;
		result += `receiptsHash: ${this._receiptsHash.toString()}, `;
		result += `stateHash: ${this._stateHash.toString()}, `;
		result += `beneficiaryAddress: ${this._beneficiaryAddress.toString()}, `;
		result += `feeMultiplier: ${this._feeMultiplier.toString()}, `;
		result += `votingEligibleAccountsCount: ${'0x'.concat(this._votingEligibleAccountsCount.toString(16))}, `;
		result += `harvestingEligibleAccountsCount: ${'0x'.concat(this._harvestingEligibleAccountsCount.toString(16))}, `;
		result += `totalVotingBalance: ${this._totalVotingBalance.toString()}, `;
		result += `previousImportanceBlockHash: ${this._previousImportanceBlockHash.toString()}, `;
		result += `transactions: [${this._transactions.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class FinalizationRound {
	static TYPE_HINTS = {
		epoch: 'pod:FinalizationEpoch',
		point: 'pod:FinalizationPoint'
	};

	constructor() {
		this._epoch = new FinalizationEpoch();
		this._point = new FinalizationPoint();
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get epoch() {
		return this._epoch;
	}

	set epoch(value) {
		this._epoch = value;
	}

	get point() {
		return this._point;
	}

	set point(value) {
		this._point = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.epoch.size;
		size += this.point.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const epoch = FinalizationEpoch.deserialize(view.buffer);
		view.shiftRight(epoch.size);
		const point = FinalizationPoint.deserialize(view.buffer);
		view.shiftRight(point.size);

		const instance = new FinalizationRound();
		instance._epoch = epoch;
		instance._point = point;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._epoch.serialize());
		buffer.write(this._point.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `epoch: ${this._epoch.toString()}, `;
		result += `point: ${this._point.toString()}, `;
		result += ')';
		return result;
	}
}

export class FinalizedBlockHeader {
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

	sort() {
		this._round.sort();
	}

	get round() {
		return this._round;
	}

	set round(value) {
		this._round = value;
	}

	get height() {
		return this._height;
	}

	set height(value) {
		this._height = value;
	}

	get hash() {
		return this._hash;
	}

	set hash(value) {
		this._hash = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.round.size;
		size += this.height.size;
		size += this.hash.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const round = FinalizationRound.deserialize(view.buffer);
		view.shiftRight(round.size);
		const height = Height.deserialize(view.buffer);
		view.shiftRight(height.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shiftRight(hash.size);

		const instance = new FinalizedBlockHeader();
		instance._round = round;
		instance._height = height;
		instance._hash = hash;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._round.serialize());
		buffer.write(this._height.serialize());
		buffer.write(this._hash.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `round: ${this._round.toString()}, `;
		result += `height: ${this._height.toString()}, `;
		result += `hash: ${this._hash.toString()}, `;
		result += ')';
		return result;
	}
}

export class ReceiptType {
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

	static valueToKey(value) {
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

		return keys[index];
	}

	static fromValue(value) {
		return ReceiptType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 2;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 2, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 2, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 2, false);
	}

	toString() {
		return `ReceiptType.${ReceiptType.valueToKey(this.value)}`;
	}
}

export class Receipt {
	static TYPE_HINTS = {
		type: 'enum:ReceiptType'
	};

	constructor() {
		this._version = 0;
		this._type = ReceiptType.MOSAIC_RENTAL_FEE;
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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
		size += 2;
		size += this.type.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);

		const instance = new Receipt();
		instance._version = version;
		instance._type = type;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);

		const instance = new Receipt();
		instance._version = version;
		instance._type = type;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += ')';
		return result;
	}
}

export class HarvestFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new HarvestFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new HarvestFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class InflationReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);

		const instance = new InflationReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);

		const instance = new InflationReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += ')';
		return result;
	}
}

export class LockHashCreatedFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockHashCreatedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockHashCreatedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class LockHashCompletedFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockHashCompletedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockHashCompletedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class LockHashExpiredFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockHashExpiredFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockHashExpiredFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class LockSecretCreatedFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockSecretCreatedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockSecretCreatedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class LockSecretCompletedFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockSecretCompletedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockSecretCompletedFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class LockSecretExpiredFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get targetAddress() {
		return this._targetAddress;
	}

	set targetAddress(value) {
		this._targetAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.targetAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockSecretExpiredFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const targetAddress = Address.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new LockSecretExpiredFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._targetAddress = targetAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._targetAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `targetAddress: ${this._targetAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicExpiredReceipt {
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

	sort() { // eslint-disable-line class-methods-use-this
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get artifactId() {
		return this._artifactId;
	}

	set artifactId(value) {
		this._artifactId = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.artifactId.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const artifactId = MosaicId.deserialize(view.buffer);
		view.shiftRight(artifactId.size);

		const instance = new MosaicExpiredReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const artifactId = MosaicId.deserializeAligned(view.buffer);
		view.shiftRight(artifactId.size);

		const instance = new MosaicExpiredReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._artifactId.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `artifactId: ${this._artifactId.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicRentalFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get senderAddress() {
		return this._senderAddress;
	}

	set senderAddress(value) {
		this._senderAddress = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.senderAddress.size;
		size += this.recipientAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const senderAddress = Address.deserialize(view.buffer);
		view.shiftRight(senderAddress.size);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);

		const instance = new MosaicRentalFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._senderAddress = senderAddress;
		instance._recipientAddress = recipientAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const senderAddress = Address.deserialize(view.buffer);
		view.shiftRight(senderAddress.size);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);

		const instance = new MosaicRentalFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._senderAddress = senderAddress;
		instance._recipientAddress = recipientAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._senderAddress.serialize());
		buffer.write(this._recipientAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `senderAddress: ${this._senderAddress.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class NamespaceId extends BaseValue {
	static SIZE = 8;

	constructor(namespaceId = 0n) {
		super(NamespaceId.SIZE, namespaceId);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new NamespaceId(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new NamespaceId(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class NamespaceRegistrationType {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class AliasAction {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class NamespaceExpiredReceipt {
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

	sort() { // eslint-disable-line class-methods-use-this
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get artifactId() {
		return this._artifactId;
	}

	set artifactId(value) {
		this._artifactId = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.artifactId.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const artifactId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(artifactId.size);

		const instance = new NamespaceExpiredReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const artifactId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(artifactId.size);

		const instance = new NamespaceExpiredReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._artifactId.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `artifactId: ${this._artifactId.toString()}, `;
		result += ')';
		return result;
	}
}

export class NamespaceDeletedReceipt {
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

	sort() { // eslint-disable-line class-methods-use-this
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
	}

	get artifactId() {
		return this._artifactId;
	}

	set artifactId(value) {
		this._artifactId = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.artifactId.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const artifactId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(artifactId.size);

		const instance = new NamespaceDeletedReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const artifactId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(artifactId.size);

		const instance = new NamespaceDeletedReceipt();
		instance._version = version;
		instance._type = type;
		instance._artifactId = artifactId;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._artifactId.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `artifactId: ${this._artifactId.toString()}, `;
		result += ')';
		return result;
	}
}

export class NamespaceRentalFeeReceipt {
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

	sort() {
		this._mosaic.sort();
	}

	get version() {
		return this._version;
	}

	set version(value) {
		this._version = value;
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

	get senderAddress() {
		return this._senderAddress;
	}

	set senderAddress(value) {
		this._senderAddress = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 2;
		size += this.type.size;
		size += this.mosaic.size;
		size += this.senderAddress.size;
		size += this.recipientAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const senderAddress = Address.deserialize(view.buffer);
		view.shiftRight(senderAddress.size);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);

		const instance = new NamespaceRentalFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._senderAddress = senderAddress;
		instance._recipientAddress = recipientAddress;
		return instance;
	}

	static deserializeAligned(payload) {
		const view = new BufferView(payload);
		const size = converter.bytesToInt(view.buffer, 4, false);
		view.shiftRight(4);
		view.shrink(size - 4);
		const version = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const type = ReceiptType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = Mosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const senderAddress = Address.deserialize(view.buffer);
		view.shiftRight(senderAddress.size);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);

		const instance = new NamespaceRentalFeeReceipt();
		instance._version = version;
		instance._type = type;
		instance._mosaic = mosaic;
		instance._senderAddress = senderAddress;
		instance._recipientAddress = recipientAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.size, 4, false));
		buffer.write(converter.intToBytes(this._version, 2, false));
		buffer.write(this._type.serialize());
		buffer.write(this._mosaic.serialize());
		buffer.write(this._senderAddress.serialize());
		buffer.write(this._recipientAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `type: ${this._type.toString()}, `;
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += `senderAddress: ${this._senderAddress.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class ReceiptSource {
	static TYPE_HINTS = {
	};

	constructor() {
		this._primaryId = 0;
		this._secondaryId = 0;
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get primaryId() {
		return this._primaryId;
	}

	set primaryId(value) {
		this._primaryId = value;
	}

	get secondaryId() {
		return this._secondaryId;
	}

	set secondaryId(value) {
		this._secondaryId = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const primaryId = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const secondaryId = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);

		const instance = new ReceiptSource();
		instance._primaryId = primaryId;
		instance._secondaryId = secondaryId;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._primaryId, 4, false));
		buffer.write(converter.intToBytes(this._secondaryId, 4, false));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `primaryId: ${'0x'.concat(this._primaryId.toString(16))}, `;
		result += `secondaryId: ${'0x'.concat(this._secondaryId.toString(16))}, `;
		result += ')';
		return result;
	}
}

export class AddressResolutionEntry {
	static TYPE_HINTS = {
		source: 'struct:ReceiptSource',
		resolvedValue: 'pod:Address'
	};

	constructor() {
		this._source = new ReceiptSource();
		this._resolvedValue = new Address();
	}

	sort() {
		this._source.sort();
	}

	get source() {
		return this._source;
	}

	set source(value) {
		this._source = value;
	}

	get resolvedValue() {
		return this._resolvedValue;
	}

	set resolvedValue(value) {
		this._resolvedValue = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.source.size;
		size += this.resolvedValue.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const source = ReceiptSource.deserialize(view.buffer);
		view.shiftRight(source.size);
		const resolvedValue = Address.deserialize(view.buffer);
		view.shiftRight(resolvedValue.size);

		const instance = new AddressResolutionEntry();
		instance._source = source;
		instance._resolvedValue = resolvedValue;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._source.serialize());
		buffer.write(this._resolvedValue.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `source: ${this._source.toString()}, `;
		result += `resolvedValue: ${this._resolvedValue.toString()}, `;
		result += ')';
		return result;
	}
}

export class AddressResolutionStatement {
	static TYPE_HINTS = {
		unresolved: 'pod:UnresolvedAddress',
		resolutionEntries: 'array[AddressResolutionEntry]'
	};

	constructor() {
		this._unresolved = new UnresolvedAddress();
		this._resolutionEntries = [];
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get unresolved() {
		return this._unresolved;
	}

	set unresolved(value) {
		this._unresolved = value;
	}

	get resolutionEntries() {
		return this._resolutionEntries;
	}

	set resolutionEntries(value) {
		this._resolutionEntries = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.unresolved.size;
		size += 4;
		size += arrayHelpers.size(this.resolutionEntries);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const unresolved = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(unresolved.size);
		const resolutionEntriesCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const resolutionEntries = arrayHelpers.readArrayCount(view.buffer, AddressResolutionEntry, resolutionEntriesCount);
		view.shiftRight(arrayHelpers.size(resolutionEntries));

		const instance = new AddressResolutionStatement();
		instance._unresolved = unresolved;
		instance._resolutionEntries = resolutionEntries;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._unresolved.serialize());
		buffer.write(converter.intToBytes(this._resolutionEntries.length, 4, false)); // bound: resolution_entries_count
		arrayHelpers.writeArray(buffer, this._resolutionEntries);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `unresolved: ${this._unresolved.toString()}, `;
		result += `resolutionEntries: [${this._resolutionEntries.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class MosaicResolutionEntry {
	static TYPE_HINTS = {
		source: 'struct:ReceiptSource',
		resolvedValue: 'pod:MosaicId'
	};

	constructor() {
		this._source = new ReceiptSource();
		this._resolvedValue = new MosaicId();
	}

	sort() {
		this._source.sort();
	}

	get source() {
		return this._source;
	}

	set source(value) {
		this._source = value;
	}

	get resolvedValue() {
		return this._resolvedValue;
	}

	set resolvedValue(value) {
		this._resolvedValue = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.source.size;
		size += this.resolvedValue.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const source = ReceiptSource.deserialize(view.buffer);
		view.shiftRight(source.size);
		const resolvedValue = MosaicId.deserialize(view.buffer);
		view.shiftRight(resolvedValue.size);

		const instance = new MosaicResolutionEntry();
		instance._source = source;
		instance._resolvedValue = resolvedValue;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._source.serialize());
		buffer.write(this._resolvedValue.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `source: ${this._source.toString()}, `;
		result += `resolvedValue: ${this._resolvedValue.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicResolutionStatement {
	static TYPE_HINTS = {
		unresolved: 'pod:UnresolvedMosaicId',
		resolutionEntries: 'array[MosaicResolutionEntry]'
	};

	constructor() {
		this._unresolved = new UnresolvedMosaicId();
		this._resolutionEntries = [];
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get unresolved() {
		return this._unresolved;
	}

	set unresolved(value) {
		this._unresolved = value;
	}

	get resolutionEntries() {
		return this._resolutionEntries;
	}

	set resolutionEntries(value) {
		this._resolutionEntries = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.unresolved.size;
		size += 4;
		size += arrayHelpers.size(this.resolutionEntries);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const unresolved = UnresolvedMosaicId.deserialize(view.buffer);
		view.shiftRight(unresolved.size);
		const resolutionEntriesCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const resolutionEntries = arrayHelpers.readArrayCount(view.buffer, MosaicResolutionEntry, resolutionEntriesCount);
		view.shiftRight(arrayHelpers.size(resolutionEntries));

		const instance = new MosaicResolutionStatement();
		instance._unresolved = unresolved;
		instance._resolutionEntries = resolutionEntries;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._unresolved.serialize());
		buffer.write(converter.intToBytes(this._resolutionEntries.length, 4, false)); // bound: resolution_entries_count
		arrayHelpers.writeArray(buffer, this._resolutionEntries);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `unresolved: ${this._unresolved.toString()}, `;
		result += `resolutionEntries: [${this._resolutionEntries.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class TransactionStatement {
	static TYPE_HINTS = {
		receipts: 'array[Receipt]'
	};

	constructor() {
		this._primaryId = 0;
		this._secondaryId = 0;
		this._receipts = [];
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get primaryId() {
		return this._primaryId;
	}

	set primaryId(value) {
		this._primaryId = value;
	}

	get secondaryId() {
		return this._secondaryId;
	}

	set secondaryId(value) {
		this._secondaryId = value;
	}

	get receipts() {
		return this._receipts;
	}

	set receipts(value) {
		this._receipts = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += 4;
		size += 4;
		size += arrayHelpers.size(this.receipts);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const primaryId = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const secondaryId = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const receiptCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const receipts = arrayHelpers.readArrayCount(view.buffer, ReceiptFactory, receiptCount);
		view.shiftRight(arrayHelpers.size(receipts));

		const instance = new TransactionStatement();
		instance._primaryId = primaryId;
		instance._secondaryId = secondaryId;
		instance._receipts = receipts;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._primaryId, 4, false));
		buffer.write(converter.intToBytes(this._secondaryId, 4, false));
		buffer.write(converter.intToBytes(this._receipts.length, 4, false)); // bound: receipt_count
		arrayHelpers.writeArray(buffer, this._receipts);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `primaryId: ${'0x'.concat(this._primaryId.toString(16))}, `;
		result += `secondaryId: ${'0x'.concat(this._secondaryId.toString(16))}, `;
		result += `receipts: [${this._receipts.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class BlockStatement {
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

	sort() { // eslint-disable-line class-methods-use-this
	}

	get transactionStatements() {
		return this._transactionStatements;
	}

	set transactionStatements(value) {
		this._transactionStatements = value;
	}

	get addressResolutionStatements() {
		return this._addressResolutionStatements;
	}

	set addressResolutionStatements(value) {
		this._addressResolutionStatements = value;
	}

	get mosaicResolutionStatements() {
		return this._mosaicResolutionStatements;
	}

	set mosaicResolutionStatements(value) {
		this._mosaicResolutionStatements = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += arrayHelpers.size(this.transactionStatements);
		size += 4;
		size += arrayHelpers.size(this.addressResolutionStatements);
		size += 4;
		size += arrayHelpers.size(this.mosaicResolutionStatements);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const transactionStatementCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const transactionStatements = arrayHelpers.readArrayCount(view.buffer, TransactionStatement, transactionStatementCount);
		view.shiftRight(arrayHelpers.size(transactionStatements));
		const addressResolutionStatementCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const addressResolutionStatements = arrayHelpers.readArrayCount(view.buffer, AddressResolutionStatement, addressResolutionStatementCount);
		view.shiftRight(arrayHelpers.size(addressResolutionStatements));
		const mosaicResolutionStatementCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const mosaicResolutionStatements = arrayHelpers.readArrayCount(view.buffer, MosaicResolutionStatement, mosaicResolutionStatementCount);
		view.shiftRight(arrayHelpers.size(mosaicResolutionStatements));

		const instance = new BlockStatement();
		instance._transactionStatements = transactionStatements;
		instance._addressResolutionStatements = addressResolutionStatements;
		instance._mosaicResolutionStatements = mosaicResolutionStatements;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._transactionStatements.length, 4, false)); // bound: transaction_statement_count
		arrayHelpers.writeArray(buffer, this._transactionStatements);
		buffer.write(converter.intToBytes(this._addressResolutionStatements.length, 4, false)); // bound: address_resolution_statement_count
		arrayHelpers.writeArray(buffer, this._addressResolutionStatements);
		buffer.write(converter.intToBytes(this._mosaicResolutionStatements.length, 4, false)); // bound: mosaic_resolution_statement_count
		arrayHelpers.writeArray(buffer, this._mosaicResolutionStatements);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `transactionStatements: [${this._transactionStatements.map(e => e.toString()).join(',')}], `;
		result += `addressResolutionStatements: [${this._addressResolutionStatements.map(e => e.toString()).join(',')}], `;
		result += `mosaicResolutionStatements: [${this._mosaicResolutionStatements.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class AccountKeyLinkTransactionV1 {
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
		this._version = AccountKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new AccountKeyLinkTransactionV1();
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

export class EmbeddedAccountKeyLinkTransactionV1 {
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
		this._version = EmbeddedAccountKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new EmbeddedAccountKeyLinkTransactionV1();
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

export class NodeKeyLinkTransactionV1 {
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
		this._version = NodeKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NodeKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new NodeKeyLinkTransactionV1();
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

export class EmbeddedNodeKeyLinkTransactionV1 {
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
		this._version = EmbeddedNodeKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedNodeKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new EmbeddedNodeKeyLinkTransactionV1();
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

export class Cosignature {
	static TYPE_HINTS = {
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature'
	};

	constructor() {
		this._version = 0n;
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const version = converter.bytesToBigInt(view.buffer, 8, false);
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

export class DetachedCosignature {
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const version = converter.bytesToBigInt(view.buffer, 8, false);
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

export class AggregateCompleteTransactionV1 {
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
		this._version = AggregateCompleteTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AggregateCompleteTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._transactionsHash = new Hash256();
		this._transactions = [];
		this._cosignatures = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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

		const instance = new AggregateCompleteTransactionV1();
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

export class AggregateCompleteTransactionV2 {
	static TRANSACTION_VERSION = 2;

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
		this._version = AggregateCompleteTransactionV2.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AggregateCompleteTransactionV2.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._transactionsHash = new Hash256();
		this._transactions = [];
		this._cosignatures = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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

		const instance = new AggregateCompleteTransactionV2();
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

export class AggregateBondedTransactionV1 {
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
		this._version = AggregateBondedTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AggregateBondedTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._transactionsHash = new Hash256();
		this._transactions = [];
		this._cosignatures = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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

		const instance = new AggregateBondedTransactionV1();
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

export class AggregateBondedTransactionV2 {
	static TRANSACTION_VERSION = 2;

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
		this._version = AggregateBondedTransactionV2.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AggregateBondedTransactionV2.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._transactionsHash = new Hash256();
		this._transactions = [];
		this._cosignatures = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._aggregateTransactionHeaderReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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

		const instance = new AggregateBondedTransactionV2();
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

export class VotingKeyLinkTransactionV1 {
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
		this._version = VotingKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = VotingKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new VotingPublicKey();
		this._startEpoch = new FinalizationEpoch();
		this._endEpoch = new FinalizationEpoch();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = VotingPublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const startEpoch = FinalizationEpoch.deserializeAligned(view.buffer);
		view.shiftRight(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserializeAligned(view.buffer);
		view.shiftRight(endEpoch.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new VotingKeyLinkTransactionV1();
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

export class EmbeddedVotingKeyLinkTransactionV1 {
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
		this._version = EmbeddedVotingKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedVotingKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._linkedPublicKey = new VotingPublicKey();
		this._startEpoch = new FinalizationEpoch();
		this._endEpoch = new FinalizationEpoch();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = VotingPublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const startEpoch = FinalizationEpoch.deserializeAligned(view.buffer);
		view.shiftRight(startEpoch.size);
		const endEpoch = FinalizationEpoch.deserializeAligned(view.buffer);
		view.shiftRight(endEpoch.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new EmbeddedVotingKeyLinkTransactionV1();
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

export class VrfKeyLinkTransactionV1 {
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
		this._version = VrfKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = VrfKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new VrfKeyLinkTransactionV1();
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

export class EmbeddedVrfKeyLinkTransactionV1 {
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
		this._version = EmbeddedVrfKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedVrfKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._linkedPublicKey = new PublicKey();
		this._linkAction = LinkAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const linkedPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(linkedPublicKey.size);
		const linkAction = LinkAction.deserializeAligned(view.buffer);
		view.shiftRight(linkAction.size);

		const instance = new EmbeddedVrfKeyLinkTransactionV1();
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

export class HashLockTransactionV1 {
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
		this._version = HashLockTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = HashLockTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hash = new Hash256();
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._mosaic.sort();
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserializeAligned(view.buffer);
		view.shiftRight(duration.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shiftRight(hash.size);

		const instance = new HashLockTransactionV1();
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

export class EmbeddedHashLockTransactionV1 {
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
		this._version = EmbeddedHashLockTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedHashLockTransactionV1.TRANSACTION_TYPE;
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hash = new Hash256();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._mosaic.sort();
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserializeAligned(view.buffer);
		view.shiftRight(duration.size);
		const hash = Hash256.deserialize(view.buffer);
		view.shiftRight(hash.size);

		const instance = new EmbeddedHashLockTransactionV1();
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

export class LockHashAlgorithm {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class SecretLockTransactionV1 {
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
		this._version = SecretLockTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = SecretLockTransactionV1.TRANSACTION_TYPE;
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

	sort() {
		this._mosaic.sort();
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserializeAligned(view.buffer);
		view.shiftRight(duration.size);
		const hashAlgorithm = LockHashAlgorithm.deserializeAligned(view.buffer);
		view.shiftRight(hashAlgorithm.size);

		const instance = new SecretLockTransactionV1();
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

export class EmbeddedSecretLockTransactionV1 {
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
		this._version = EmbeddedSecretLockTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedSecretLockTransactionV1.TRANSACTION_TYPE;
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._mosaic = new UnresolvedMosaic();
		this._duration = new BlockDuration();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._mosaic.sort();
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);
		const duration = BlockDuration.deserializeAligned(view.buffer);
		view.shiftRight(duration.size);
		const hashAlgorithm = LockHashAlgorithm.deserializeAligned(view.buffer);
		view.shiftRight(hashAlgorithm.size);

		const instance = new EmbeddedSecretLockTransactionV1();
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

export class SecretProofTransactionV1 {
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
		this._version = SecretProofTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = SecretProofTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._proof = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const proofSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const hashAlgorithm = LockHashAlgorithm.deserializeAligned(view.buffer);
		view.shiftRight(hashAlgorithm.size);
		const proof = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, proofSize);
		view.shiftRight(proofSize);

		const instance = new SecretProofTransactionV1();
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

export class EmbeddedSecretProofTransactionV1 {
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
		this._version = EmbeddedSecretProofTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedSecretProofTransactionV1.TRANSACTION_TYPE;
		this._recipientAddress = new UnresolvedAddress();
		this._secret = new Hash256();
		this._hashAlgorithm = LockHashAlgorithm.SHA3_256;
		this._proof = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const recipientAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const secret = Hash256.deserialize(view.buffer);
		view.shiftRight(secret.size);
		const proofSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const hashAlgorithm = LockHashAlgorithm.deserializeAligned(view.buffer);
		view.shiftRight(hashAlgorithm.size);
		const proof = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, proofSize);
		view.shiftRight(proofSize);

		const instance = new EmbeddedSecretProofTransactionV1();
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

export class AccountMetadataTransactionV1 {
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
		this._version = AccountMetadataTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountMetadataTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new AccountMetadataTransactionV1();
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

export class EmbeddedAccountMetadataTransactionV1 {
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
		this._version = EmbeddedAccountMetadataTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountMetadataTransactionV1.TRANSACTION_TYPE;
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new EmbeddedAccountMetadataTransactionV1();
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

export class MosaicMetadataTransactionV1 {
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
		this._version = MosaicMetadataTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicMetadataTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetMosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(targetMosaicId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new MosaicMetadataTransactionV1();
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

export class EmbeddedMosaicMetadataTransactionV1 {
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
		this._version = EmbeddedMosaicMetadataTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicMetadataTransactionV1.TRANSACTION_TYPE;
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._targetMosaicId = new UnresolvedMosaicId();
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetMosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(targetMosaicId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new EmbeddedMosaicMetadataTransactionV1();
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

export class NamespaceMetadataTransactionV1 {
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
		this._version = NamespaceMetadataTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NamespaceMetadataTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetNamespaceId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(targetNamespaceId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new NamespaceMetadataTransactionV1();
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

export class EmbeddedNamespaceMetadataTransactionV1 {
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
		this._version = EmbeddedNamespaceMetadataTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedNamespaceMetadataTransactionV1.TRANSACTION_TYPE;
		this._targetAddress = new UnresolvedAddress();
		this._scopedMetadataKey = 0n;
		this._targetNamespaceId = new NamespaceId();
		this._valueSizeDelta = 0;
		this._value = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);
		const scopedMetadataKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetNamespaceId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(targetNamespaceId.size);
		const valueSizeDelta = converter.bytesToInt(view.buffer, 2, true);
		view.shiftRight(2);
		const valueSize = converter.bytesToInt(view.buffer, 2, false);
		view.shiftRight(2);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new EmbeddedNamespaceMetadataTransactionV1();
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

export class MosaicNonce extends BaseValue {
	static SIZE = 4;

	constructor(mosaicNonce = 0) {
		super(MosaicNonce.SIZE, mosaicNonce);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicNonce(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new MosaicNonce(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

export class MosaicFlags {
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
		return new MosaicFlags(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class MosaicSupplyChangeAction {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class MosaicDefinitionTransactionV1 {
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
		this._version = MosaicDefinitionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicDefinitionTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const id = MosaicId.deserializeAligned(view.buffer);
		view.shiftRight(id.size);
		const duration = BlockDuration.deserializeAligned(view.buffer);
		view.shiftRight(duration.size);
		const nonce = MosaicNonce.deserializeAligned(view.buffer);
		view.shiftRight(nonce.size);
		const flags = MosaicFlags.deserializeAligned(view.buffer);
		view.shiftRight(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);

		const instance = new MosaicDefinitionTransactionV1();
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

export class EmbeddedMosaicDefinitionTransactionV1 {
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
		this._version = EmbeddedMosaicDefinitionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicDefinitionTransactionV1.TRANSACTION_TYPE;
		this._id = new MosaicId();
		this._duration = new BlockDuration();
		this._nonce = new MosaicNonce();
		this._flags = MosaicFlags.NONE;
		this._divisibility = 0;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const id = MosaicId.deserializeAligned(view.buffer);
		view.shiftRight(id.size);
		const duration = BlockDuration.deserializeAligned(view.buffer);
		view.shiftRight(duration.size);
		const nonce = MosaicNonce.deserializeAligned(view.buffer);
		view.shiftRight(nonce.size);
		const flags = MosaicFlags.deserializeAligned(view.buffer);
		view.shiftRight(flags.size);
		const divisibility = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);

		const instance = new EmbeddedMosaicDefinitionTransactionV1();
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

export class MosaicSupplyChangeTransactionV1 {
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
		this._version = MosaicSupplyChangeTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicSupplyChangeTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicId = new UnresolvedMosaicId();
		this._delta = new Amount();
		this._action = MosaicSupplyChangeAction.DECREASE;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const delta = Amount.deserializeAligned(view.buffer);
		view.shiftRight(delta.size);
		const action = MosaicSupplyChangeAction.deserializeAligned(view.buffer);
		view.shiftRight(action.size);

		const instance = new MosaicSupplyChangeTransactionV1();
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

export class EmbeddedMosaicSupplyChangeTransactionV1 {
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
		this._version = EmbeddedMosaicSupplyChangeTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicSupplyChangeTransactionV1.TRANSACTION_TYPE;
		this._mosaicId = new UnresolvedMosaicId();
		this._delta = new Amount();
		this._action = MosaicSupplyChangeAction.DECREASE;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const delta = Amount.deserializeAligned(view.buffer);
		view.shiftRight(delta.size);
		const action = MosaicSupplyChangeAction.deserializeAligned(view.buffer);
		view.shiftRight(action.size);

		const instance = new EmbeddedMosaicSupplyChangeTransactionV1();
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

export class MosaicSupplyRevocationTransactionV1 {
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
		this._version = MosaicSupplyRevocationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicSupplyRevocationTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._sourceAddress = new UnresolvedAddress();
		this._mosaic = new UnresolvedMosaic();
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._mosaic.sort();
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const sourceAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(sourceAddress.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);

		const instance = new MosaicSupplyRevocationTransactionV1();
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

export class EmbeddedMosaicSupplyRevocationTransactionV1 {
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
		this._version = EmbeddedMosaicSupplyRevocationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicSupplyRevocationTransactionV1.TRANSACTION_TYPE;
		this._sourceAddress = new UnresolvedAddress();
		this._mosaic = new UnresolvedMosaic();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() {
		this._mosaic.sort();
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const sourceAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(sourceAddress.size);
		const mosaic = UnresolvedMosaic.deserialize(view.buffer);
		view.shiftRight(mosaic.size);

		const instance = new EmbeddedMosaicSupplyRevocationTransactionV1();
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

export class MultisigAccountModificationTransactionV1 {
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
		this._version = MultisigAccountModificationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MultisigAccountModificationTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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

		const instance = new MultisigAccountModificationTransactionV1();
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

export class EmbeddedMultisigAccountModificationTransactionV1 {
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
		this._version = EmbeddedMultisigAccountModificationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMultisigAccountModificationTransactionV1.TRANSACTION_TYPE;
		this._minRemovalDelta = 0;
		this._minApprovalDelta = 0;
		this._addressAdditions = [];
		this._addressDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._multisigAccountModificationTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
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

		const instance = new EmbeddedMultisigAccountModificationTransactionV1();
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

export class AddressAliasTransactionV1 {
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
		this._version = AddressAliasTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AddressAliasTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._namespaceId = new NamespaceId();
		this._address = new Address();
		this._aliasAction = AliasAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const namespaceId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(namespaceId.size);
		const address = Address.deserialize(view.buffer);
		view.shiftRight(address.size);
		const aliasAction = AliasAction.deserializeAligned(view.buffer);
		view.shiftRight(aliasAction.size);

		const instance = new AddressAliasTransactionV1();
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

export class EmbeddedAddressAliasTransactionV1 {
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
		this._version = EmbeddedAddressAliasTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAddressAliasTransactionV1.TRANSACTION_TYPE;
		this._namespaceId = new NamespaceId();
		this._address = new Address();
		this._aliasAction = AliasAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const namespaceId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(namespaceId.size);
		const address = Address.deserialize(view.buffer);
		view.shiftRight(address.size);
		const aliasAction = AliasAction.deserializeAligned(view.buffer);
		view.shiftRight(aliasAction.size);

		const instance = new EmbeddedAddressAliasTransactionV1();
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

export class MosaicAliasTransactionV1 {
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
		this._version = MosaicAliasTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicAliasTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._namespaceId = new NamespaceId();
		this._mosaicId = new MosaicId();
		this._aliasAction = AliasAction.UNLINK;
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const namespaceId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(namespaceId.size);
		const mosaicId = MosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const aliasAction = AliasAction.deserializeAligned(view.buffer);
		view.shiftRight(aliasAction.size);

		const instance = new MosaicAliasTransactionV1();
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

export class EmbeddedMosaicAliasTransactionV1 {
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
		this._version = EmbeddedMosaicAliasTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicAliasTransactionV1.TRANSACTION_TYPE;
		this._namespaceId = new NamespaceId();
		this._mosaicId = new MosaicId();
		this._aliasAction = AliasAction.UNLINK;
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const namespaceId = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(namespaceId.size);
		const mosaicId = MosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const aliasAction = AliasAction.deserializeAligned(view.buffer);
		view.shiftRight(aliasAction.size);

		const instance = new EmbeddedMosaicAliasTransactionV1();
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

export class NamespaceRegistrationTransactionV1 {
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
		this._version = NamespaceRegistrationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = NamespaceRegistrationTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._duration = new BlockDuration();
		this._parentId = null;
		this._id = new NamespaceId();
		this._registrationType = NamespaceRegistrationType.ROOT;
		this._name = new Uint8Array();
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		// deserialize to temporary buffer for further processing
		const durationTemporary = BlockDuration.deserialize(view.buffer);
		const registration_type_condition = view.window(durationTemporary.size);
		view.shiftRight(durationTemporary.size); // skip temporary

		const id = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(id.size);
		const registrationType = NamespaceRegistrationType.deserializeAligned(view.buffer);
		view.shiftRight(registrationType.size);
		let duration = new BlockDuration();
		if (NamespaceRegistrationType.ROOT === registrationType)
			duration = BlockDuration.deserializeAligned(registration_type_condition);

		let parentId = new NamespaceId();
		if (NamespaceRegistrationType.CHILD === registrationType)
			parentId = NamespaceId.deserializeAligned(registration_type_condition);

		const nameSize = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);

		const instance = new NamespaceRegistrationTransactionV1();
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

export class EmbeddedNamespaceRegistrationTransactionV1 {
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
		this._version = EmbeddedNamespaceRegistrationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedNamespaceRegistrationTransactionV1.TRANSACTION_TYPE;
		this._duration = new BlockDuration();
		this._parentId = null;
		this._id = new NamespaceId();
		this._registrationType = NamespaceRegistrationType.ROOT;
		this._name = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		// deserialize to temporary buffer for further processing
		const durationTemporary = BlockDuration.deserialize(view.buffer);
		const registration_type_condition = view.window(durationTemporary.size);
		view.shiftRight(durationTemporary.size); // skip temporary

		const id = NamespaceId.deserializeAligned(view.buffer);
		view.shiftRight(id.size);
		const registrationType = NamespaceRegistrationType.deserializeAligned(view.buffer);
		view.shiftRight(registrationType.size);
		let duration = new BlockDuration();
		if (NamespaceRegistrationType.ROOT === registrationType)
			duration = BlockDuration.deserializeAligned(registration_type_condition);

		let parentId = new NamespaceId();
		if (NamespaceRegistrationType.CHILD === registrationType)
			parentId = NamespaceId.deserializeAligned(registration_type_condition);

		const nameSize = converter.bytesToInt(view.buffer, 1, false);
		view.shiftRight(1);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);

		const instance = new EmbeddedNamespaceRegistrationTransactionV1();
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

export class AccountRestrictionFlags {
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
		return new AccountRestrictionFlags(converter.bytesToIntUnaligned(byteArray, 2, false));
	}

	static deserializeAligned(payload) {
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

export class AccountAddressRestrictionTransactionV1 {
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
		this._version = AccountAddressRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountAddressRestrictionTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserializeAligned(view.buffer);
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

		const instance = new AccountAddressRestrictionTransactionV1();
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

export class EmbeddedAccountAddressRestrictionTransactionV1 {
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
		this._version = EmbeddedAccountAddressRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountAddressRestrictionTransactionV1.TRANSACTION_TYPE;
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserializeAligned(view.buffer);
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

		const instance = new EmbeddedAccountAddressRestrictionTransactionV1();
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

export class AccountMosaicRestrictionTransactionV1 {
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
		this._version = AccountMosaicRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountMosaicRestrictionTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserializeAligned(view.buffer);
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

		const instance = new AccountMosaicRestrictionTransactionV1();
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

export class EmbeddedAccountMosaicRestrictionTransactionV1 {
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
		this._version = EmbeddedAccountMosaicRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountMosaicRestrictionTransactionV1.TRANSACTION_TYPE;
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserializeAligned(view.buffer);
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

		const instance = new EmbeddedAccountMosaicRestrictionTransactionV1();
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

export class AccountOperationRestrictionTransactionV1 {
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
		this._version = AccountOperationRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = AccountOperationRestrictionTransactionV1.TRANSACTION_TYPE;
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._verifiableEntityHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const restrictionFlags = AccountRestrictionFlags.deserializeAligned(view.buffer);
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

		const instance = new AccountOperationRestrictionTransactionV1();
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

export class EmbeddedAccountOperationRestrictionTransactionV1 {
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
		this._version = EmbeddedAccountOperationRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedAccountOperationRestrictionTransactionV1.TRANSACTION_TYPE;
		this._restrictionFlags = AccountRestrictionFlags.ADDRESS;
		this._restrictionAdditions = [];
		this._restrictionDeletions = [];
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._accountRestrictionTransactionBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const restrictionFlags = AccountRestrictionFlags.deserializeAligned(view.buffer);
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

		const instance = new EmbeddedAccountOperationRestrictionTransactionV1();
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

export class MosaicAddressRestrictionTransactionV1 {
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
		this._version = MosaicAddressRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicAddressRestrictionTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const restrictionKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new MosaicAddressRestrictionTransactionV1();
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

export class EmbeddedMosaicAddressRestrictionTransactionV1 {
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
		this._version = EmbeddedMosaicAddressRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicAddressRestrictionTransactionV1.TRANSACTION_TYPE;
		this._mosaicId = new UnresolvedMosaicId();
		this._restrictionKey = 0n;
		this._previousRestrictionValue = 0n;
		this._newRestrictionValue = 0n;
		this._targetAddress = new UnresolvedAddress();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const restrictionKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const targetAddress = UnresolvedAddress.deserialize(view.buffer);
		view.shiftRight(targetAddress.size);

		const instance = new EmbeddedMosaicAddressRestrictionTransactionV1();
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

export class MosaicRestrictionKey extends BaseValue {
	static SIZE = 8;

	constructor(mosaicRestrictionKey = 0n) {
		super(MosaicRestrictionKey.SIZE, mosaicRestrictionKey);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new MosaicRestrictionKey(converter.bytesToBigIntUnaligned(byteArray, 8, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new MosaicRestrictionKey(converter.bytesToBigInt(byteArray, 8, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 8, false);
	}
}

export class MosaicRestrictionType {
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
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 1, false));
	}

	static deserializeAligned(payload) {
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

export class MosaicGlobalRestrictionTransactionV1 {
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
		this._version = MosaicGlobalRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = MosaicGlobalRestrictionTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const referenceMosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(referenceMosaicId.size);
		const restrictionKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionType = MosaicRestrictionType.deserializeAligned(view.buffer);
		view.shiftRight(previousRestrictionType.size);
		const newRestrictionType = MosaicRestrictionType.deserializeAligned(view.buffer);
		view.shiftRight(newRestrictionType.size);

		const instance = new MosaicGlobalRestrictionTransactionV1();
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

export class EmbeddedMosaicGlobalRestrictionTransactionV1 {
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
		this._version = EmbeddedMosaicGlobalRestrictionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedMosaicGlobalRestrictionTransactionV1.TRANSACTION_TYPE;
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

	sort() { // eslint-disable-line class-methods-use-this
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const mosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(mosaicId.size);
		const referenceMosaicId = UnresolvedMosaicId.deserializeAligned(view.buffer);
		view.shiftRight(referenceMosaicId.size);
		const restrictionKey = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const newRestrictionValue = converter.bytesToBigInt(view.buffer, 8, false);
		view.shiftRight(8);
		const previousRestrictionType = MosaicRestrictionType.deserializeAligned(view.buffer);
		view.shiftRight(previousRestrictionType.size);
		const newRestrictionType = MosaicRestrictionType.deserializeAligned(view.buffer);
		view.shiftRight(newRestrictionType.size);

		const instance = new EmbeddedMosaicGlobalRestrictionTransactionV1();
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

export class TransferTransactionV1 {
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
		this._version = TransferTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = TransferTransactionV1.TRANSACTION_TYPE;
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

	sort() {
		this._mosaics = this._mosaics.sort((lhs, rhs) => arrayHelpers.deepCompare(
			(lhs.mosaicId.comparer ? lhs.mosaicId.comparer() : lhs.mosaicId.value),
			(rhs.mosaicId.comparer ? rhs.mosaicId.comparer() : rhs.mosaicId.value)
		));
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
		view.shiftRight(type.size);
		const fee = Amount.deserializeAligned(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserializeAligned(view.buffer);
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
		const mosaics = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaic, mosaicsCount, e => ((e.mosaicId.comparer ? e.mosaicId.comparer() : e.mosaicId.value)));
		view.shiftRight(arrayHelpers.size(mosaics));
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shiftRight(messageSize);

		const instance = new TransferTransactionV1();
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
		arrayHelpers.writeArray(buffer, this._mosaics, e => ((e.mosaicId.comparer ? e.mosaicId.comparer() : e.mosaicId.value)));
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

export class EmbeddedTransferTransactionV1 {
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
		this._version = EmbeddedTransferTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._type = EmbeddedTransferTransactionV1.TRANSACTION_TYPE;
		this._recipientAddress = new UnresolvedAddress();
		this._mosaics = [];
		this._message = new Uint8Array();
		this._embeddedTransactionHeaderReserved_1 = 0; // reserved field
		this._entityBodyReserved_1 = 0; // reserved field
		this._transferTransactionBodyReserved_1 = 0; // reserved field
		this._transferTransactionBodyReserved_2 = 0; // reserved field
	}

	sort() {
		this._mosaics = this._mosaics.sort((lhs, rhs) => arrayHelpers.deepCompare(
			(lhs.mosaicId.comparer ? lhs.mosaicId.comparer() : lhs.mosaicId.value),
			(rhs.mosaicId.comparer ? rhs.mosaicId.comparer() : rhs.mosaicId.value)
		));
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
		const network = NetworkType.deserializeAligned(view.buffer);
		view.shiftRight(network.size);
		const type = TransactionType.deserializeAligned(view.buffer);
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
		const mosaics = arrayHelpers.readArrayCount(view.buffer, UnresolvedMosaic, mosaicsCount, e => ((e.mosaicId.comparer ? e.mosaicId.comparer() : e.mosaicId.value)));
		view.shiftRight(arrayHelpers.size(mosaics));
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shiftRight(messageSize);

		const instance = new EmbeddedTransferTransactionV1();
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
		arrayHelpers.writeArray(buffer, this._mosaics, e => ((e.mosaicId.comparer ? e.mosaicId.comparer() : e.mosaicId.value)));
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

export class TransactionFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = Transaction.deserialize(view.buffer);
		const mapping = new Map();
		mapping.set(TransactionFactory.toKey([AccountKeyLinkTransactionV1.TRANSACTION_TYPE.value, AccountKeyLinkTransactionV1.TRANSACTION_VERSION]), AccountKeyLinkTransactionV1);
		mapping.set(TransactionFactory.toKey([NodeKeyLinkTransactionV1.TRANSACTION_TYPE.value, NodeKeyLinkTransactionV1.TRANSACTION_VERSION]), NodeKeyLinkTransactionV1);
		mapping.set(TransactionFactory.toKey([AggregateCompleteTransactionV1.TRANSACTION_TYPE.value, AggregateCompleteTransactionV1.TRANSACTION_VERSION]), AggregateCompleteTransactionV1);
		mapping.set(TransactionFactory.toKey([AggregateCompleteTransactionV2.TRANSACTION_TYPE.value, AggregateCompleteTransactionV2.TRANSACTION_VERSION]), AggregateCompleteTransactionV2);
		mapping.set(TransactionFactory.toKey([AggregateBondedTransactionV1.TRANSACTION_TYPE.value, AggregateBondedTransactionV1.TRANSACTION_VERSION]), AggregateBondedTransactionV1);
		mapping.set(TransactionFactory.toKey([AggregateBondedTransactionV2.TRANSACTION_TYPE.value, AggregateBondedTransactionV2.TRANSACTION_VERSION]), AggregateBondedTransactionV2);
		mapping.set(TransactionFactory.toKey([VotingKeyLinkTransactionV1.TRANSACTION_TYPE.value, VotingKeyLinkTransactionV1.TRANSACTION_VERSION]), VotingKeyLinkTransactionV1);
		mapping.set(TransactionFactory.toKey([VrfKeyLinkTransactionV1.TRANSACTION_TYPE.value, VrfKeyLinkTransactionV1.TRANSACTION_VERSION]), VrfKeyLinkTransactionV1);
		mapping.set(TransactionFactory.toKey([HashLockTransactionV1.TRANSACTION_TYPE.value, HashLockTransactionV1.TRANSACTION_VERSION]), HashLockTransactionV1);
		mapping.set(TransactionFactory.toKey([SecretLockTransactionV1.TRANSACTION_TYPE.value, SecretLockTransactionV1.TRANSACTION_VERSION]), SecretLockTransactionV1);
		mapping.set(TransactionFactory.toKey([SecretProofTransactionV1.TRANSACTION_TYPE.value, SecretProofTransactionV1.TRANSACTION_VERSION]), SecretProofTransactionV1);
		mapping.set(TransactionFactory.toKey([AccountMetadataTransactionV1.TRANSACTION_TYPE.value, AccountMetadataTransactionV1.TRANSACTION_VERSION]), AccountMetadataTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicMetadataTransactionV1.TRANSACTION_TYPE.value, MosaicMetadataTransactionV1.TRANSACTION_VERSION]), MosaicMetadataTransactionV1);
		mapping.set(TransactionFactory.toKey([NamespaceMetadataTransactionV1.TRANSACTION_TYPE.value, NamespaceMetadataTransactionV1.TRANSACTION_VERSION]), NamespaceMetadataTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicDefinitionTransactionV1.TRANSACTION_TYPE.value, MosaicDefinitionTransactionV1.TRANSACTION_VERSION]), MosaicDefinitionTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicSupplyChangeTransactionV1.TRANSACTION_TYPE.value, MosaicSupplyChangeTransactionV1.TRANSACTION_VERSION]), MosaicSupplyChangeTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicSupplyRevocationTransactionV1.TRANSACTION_TYPE.value, MosaicSupplyRevocationTransactionV1.TRANSACTION_VERSION]), MosaicSupplyRevocationTransactionV1);
		mapping.set(TransactionFactory.toKey([MultisigAccountModificationTransactionV1.TRANSACTION_TYPE.value, MultisigAccountModificationTransactionV1.TRANSACTION_VERSION]), MultisigAccountModificationTransactionV1);
		mapping.set(TransactionFactory.toKey([AddressAliasTransactionV1.TRANSACTION_TYPE.value, AddressAliasTransactionV1.TRANSACTION_VERSION]), AddressAliasTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicAliasTransactionV1.TRANSACTION_TYPE.value, MosaicAliasTransactionV1.TRANSACTION_VERSION]), MosaicAliasTransactionV1);
		mapping.set(TransactionFactory.toKey([NamespaceRegistrationTransactionV1.TRANSACTION_TYPE.value, NamespaceRegistrationTransactionV1.TRANSACTION_VERSION]), NamespaceRegistrationTransactionV1);
		mapping.set(TransactionFactory.toKey([AccountAddressRestrictionTransactionV1.TRANSACTION_TYPE.value, AccountAddressRestrictionTransactionV1.TRANSACTION_VERSION]), AccountAddressRestrictionTransactionV1);
		mapping.set(TransactionFactory.toKey([AccountMosaicRestrictionTransactionV1.TRANSACTION_TYPE.value, AccountMosaicRestrictionTransactionV1.TRANSACTION_VERSION]), AccountMosaicRestrictionTransactionV1);
		mapping.set(TransactionFactory.toKey([AccountOperationRestrictionTransactionV1.TRANSACTION_TYPE.value, AccountOperationRestrictionTransactionV1.TRANSACTION_VERSION]), AccountOperationRestrictionTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicAddressRestrictionTransactionV1.TRANSACTION_TYPE.value, MosaicAddressRestrictionTransactionV1.TRANSACTION_VERSION]), MosaicAddressRestrictionTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicGlobalRestrictionTransactionV1.TRANSACTION_TYPE.value, MosaicGlobalRestrictionTransactionV1.TRANSACTION_VERSION]), MosaicGlobalRestrictionTransactionV1);
		mapping.set(TransactionFactory.toKey([TransferTransactionV1.TRANSACTION_TYPE.value, TransferTransactionV1.TRANSACTION_VERSION]), TransferTransactionV1);
		const discriminator = TransactionFactory.toKey([parent.type.value, parent.version]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			account_key_link_transaction_v1: AccountKeyLinkTransactionV1,
			node_key_link_transaction_v1: NodeKeyLinkTransactionV1,
			aggregate_complete_transaction_v1: AggregateCompleteTransactionV1,
			aggregate_complete_transaction_v2: AggregateCompleteTransactionV2,
			aggregate_bonded_transaction_v1: AggregateBondedTransactionV1,
			aggregate_bonded_transaction_v2: AggregateBondedTransactionV2,
			voting_key_link_transaction_v1: VotingKeyLinkTransactionV1,
			vrf_key_link_transaction_v1: VrfKeyLinkTransactionV1,
			hash_lock_transaction_v1: HashLockTransactionV1,
			secret_lock_transaction_v1: SecretLockTransactionV1,
			secret_proof_transaction_v1: SecretProofTransactionV1,
			account_metadata_transaction_v1: AccountMetadataTransactionV1,
			mosaic_metadata_transaction_v1: MosaicMetadataTransactionV1,
			namespace_metadata_transaction_v1: NamespaceMetadataTransactionV1,
			mosaic_definition_transaction_v1: MosaicDefinitionTransactionV1,
			mosaic_supply_change_transaction_v1: MosaicSupplyChangeTransactionV1,
			mosaic_supply_revocation_transaction_v1: MosaicSupplyRevocationTransactionV1,
			multisig_account_modification_transaction_v1: MultisigAccountModificationTransactionV1,
			address_alias_transaction_v1: AddressAliasTransactionV1,
			mosaic_alias_transaction_v1: MosaicAliasTransactionV1,
			namespace_registration_transaction_v1: NamespaceRegistrationTransactionV1,
			account_address_restriction_transaction_v1: AccountAddressRestrictionTransactionV1,
			account_mosaic_restriction_transaction_v1: AccountMosaicRestrictionTransactionV1,
			account_operation_restriction_transaction_v1: AccountOperationRestrictionTransactionV1,
			mosaic_address_restriction_transaction_v1: MosaicAddressRestrictionTransactionV1,
			mosaic_global_restriction_transaction_v1: MosaicGlobalRestrictionTransactionV1,
			transfer_transaction_v1: TransferTransactionV1
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError(`unknown Transaction type ${entityName}`);

		return new mapping[entityName]();
	}
}

export class EmbeddedTransactionFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = EmbeddedTransaction.deserialize(view.buffer);
		const mapping = new Map();
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedAccountKeyLinkTransactionV1.TRANSACTION_TYPE.value, EmbeddedAccountKeyLinkTransactionV1.TRANSACTION_VERSION]), EmbeddedAccountKeyLinkTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedNodeKeyLinkTransactionV1.TRANSACTION_TYPE.value, EmbeddedNodeKeyLinkTransactionV1.TRANSACTION_VERSION]), EmbeddedNodeKeyLinkTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedVotingKeyLinkTransactionV1.TRANSACTION_TYPE.value, EmbeddedVotingKeyLinkTransactionV1.TRANSACTION_VERSION]), EmbeddedVotingKeyLinkTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedVrfKeyLinkTransactionV1.TRANSACTION_TYPE.value, EmbeddedVrfKeyLinkTransactionV1.TRANSACTION_VERSION]), EmbeddedVrfKeyLinkTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedHashLockTransactionV1.TRANSACTION_TYPE.value, EmbeddedHashLockTransactionV1.TRANSACTION_VERSION]), EmbeddedHashLockTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedSecretLockTransactionV1.TRANSACTION_TYPE.value, EmbeddedSecretLockTransactionV1.TRANSACTION_VERSION]), EmbeddedSecretLockTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedSecretProofTransactionV1.TRANSACTION_TYPE.value, EmbeddedSecretProofTransactionV1.TRANSACTION_VERSION]), EmbeddedSecretProofTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedAccountMetadataTransactionV1.TRANSACTION_TYPE.value, EmbeddedAccountMetadataTransactionV1.TRANSACTION_VERSION]), EmbeddedAccountMetadataTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicMetadataTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicMetadataTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicMetadataTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedNamespaceMetadataTransactionV1.TRANSACTION_TYPE.value, EmbeddedNamespaceMetadataTransactionV1.TRANSACTION_VERSION]), EmbeddedNamespaceMetadataTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicDefinitionTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicDefinitionTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicDefinitionTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicSupplyChangeTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicSupplyChangeTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicSupplyChangeTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicSupplyRevocationTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicSupplyRevocationTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicSupplyRevocationTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMultisigAccountModificationTransactionV1.TRANSACTION_TYPE.value, EmbeddedMultisigAccountModificationTransactionV1.TRANSACTION_VERSION]), EmbeddedMultisigAccountModificationTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedAddressAliasTransactionV1.TRANSACTION_TYPE.value, EmbeddedAddressAliasTransactionV1.TRANSACTION_VERSION]), EmbeddedAddressAliasTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicAliasTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicAliasTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicAliasTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedNamespaceRegistrationTransactionV1.TRANSACTION_TYPE.value, EmbeddedNamespaceRegistrationTransactionV1.TRANSACTION_VERSION]), EmbeddedNamespaceRegistrationTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedAccountAddressRestrictionTransactionV1.TRANSACTION_TYPE.value, EmbeddedAccountAddressRestrictionTransactionV1.TRANSACTION_VERSION]), EmbeddedAccountAddressRestrictionTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedAccountMosaicRestrictionTransactionV1.TRANSACTION_TYPE.value, EmbeddedAccountMosaicRestrictionTransactionV1.TRANSACTION_VERSION]), EmbeddedAccountMosaicRestrictionTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedAccountOperationRestrictionTransactionV1.TRANSACTION_TYPE.value, EmbeddedAccountOperationRestrictionTransactionV1.TRANSACTION_VERSION]), EmbeddedAccountOperationRestrictionTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicAddressRestrictionTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicAddressRestrictionTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicAddressRestrictionTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedMosaicGlobalRestrictionTransactionV1.TRANSACTION_TYPE.value, EmbeddedMosaicGlobalRestrictionTransactionV1.TRANSACTION_VERSION]), EmbeddedMosaicGlobalRestrictionTransactionV1);
		mapping.set(EmbeddedTransactionFactory.toKey([EmbeddedTransferTransactionV1.TRANSACTION_TYPE.value, EmbeddedTransferTransactionV1.TRANSACTION_VERSION]), EmbeddedTransferTransactionV1);
		const discriminator = EmbeddedTransactionFactory.toKey([parent.type.value, parent.version]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			account_key_link_transaction_v1: EmbeddedAccountKeyLinkTransactionV1,
			node_key_link_transaction_v1: EmbeddedNodeKeyLinkTransactionV1,
			voting_key_link_transaction_v1: EmbeddedVotingKeyLinkTransactionV1,
			vrf_key_link_transaction_v1: EmbeddedVrfKeyLinkTransactionV1,
			hash_lock_transaction_v1: EmbeddedHashLockTransactionV1,
			secret_lock_transaction_v1: EmbeddedSecretLockTransactionV1,
			secret_proof_transaction_v1: EmbeddedSecretProofTransactionV1,
			account_metadata_transaction_v1: EmbeddedAccountMetadataTransactionV1,
			mosaic_metadata_transaction_v1: EmbeddedMosaicMetadataTransactionV1,
			namespace_metadata_transaction_v1: EmbeddedNamespaceMetadataTransactionV1,
			mosaic_definition_transaction_v1: EmbeddedMosaicDefinitionTransactionV1,
			mosaic_supply_change_transaction_v1: EmbeddedMosaicSupplyChangeTransactionV1,
			mosaic_supply_revocation_transaction_v1: EmbeddedMosaicSupplyRevocationTransactionV1,
			multisig_account_modification_transaction_v1: EmbeddedMultisigAccountModificationTransactionV1,
			address_alias_transaction_v1: EmbeddedAddressAliasTransactionV1,
			mosaic_alias_transaction_v1: EmbeddedMosaicAliasTransactionV1,
			namespace_registration_transaction_v1: EmbeddedNamespaceRegistrationTransactionV1,
			account_address_restriction_transaction_v1: EmbeddedAccountAddressRestrictionTransactionV1,
			account_mosaic_restriction_transaction_v1: EmbeddedAccountMosaicRestrictionTransactionV1,
			account_operation_restriction_transaction_v1: EmbeddedAccountOperationRestrictionTransactionV1,
			mosaic_address_restriction_transaction_v1: EmbeddedMosaicAddressRestrictionTransactionV1,
			mosaic_global_restriction_transaction_v1: EmbeddedMosaicGlobalRestrictionTransactionV1,
			transfer_transaction_v1: EmbeddedTransferTransactionV1
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError(`unknown EmbeddedTransaction type ${entityName}`);

		return new mapping[entityName]();
	}
}

export class BlockFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = Block.deserialize(view.buffer);
		const mapping = new Map();
		mapping.set(BlockFactory.toKey([NemesisBlockV1.BLOCK_TYPE.value]), NemesisBlockV1);
		mapping.set(BlockFactory.toKey([NormalBlockV1.BLOCK_TYPE.value]), NormalBlockV1);
		mapping.set(BlockFactory.toKey([ImportanceBlockV1.BLOCK_TYPE.value]), ImportanceBlockV1);
		const discriminator = BlockFactory.toKey([parent.type.value]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			nemesis_block_v1: NemesisBlockV1,
			normal_block_v1: NormalBlockV1,
			importance_block_v1: ImportanceBlockV1
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError(`unknown Block type ${entityName}`);

		return new mapping[entityName]();
	}
}

export class ReceiptFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = Receipt.deserialize(view.buffer);
		const mapping = new Map();
		mapping.set(ReceiptFactory.toKey([HarvestFeeReceipt.RECEIPT_TYPE.value]), HarvestFeeReceipt);
		mapping.set(ReceiptFactory.toKey([InflationReceipt.RECEIPT_TYPE.value]), InflationReceipt);
		mapping.set(ReceiptFactory.toKey([LockHashCreatedFeeReceipt.RECEIPT_TYPE.value]), LockHashCreatedFeeReceipt);
		mapping.set(ReceiptFactory.toKey([LockHashCompletedFeeReceipt.RECEIPT_TYPE.value]), LockHashCompletedFeeReceipt);
		mapping.set(ReceiptFactory.toKey([LockHashExpiredFeeReceipt.RECEIPT_TYPE.value]), LockHashExpiredFeeReceipt);
		mapping.set(ReceiptFactory.toKey([LockSecretCreatedFeeReceipt.RECEIPT_TYPE.value]), LockSecretCreatedFeeReceipt);
		mapping.set(ReceiptFactory.toKey([LockSecretCompletedFeeReceipt.RECEIPT_TYPE.value]), LockSecretCompletedFeeReceipt);
		mapping.set(ReceiptFactory.toKey([LockSecretExpiredFeeReceipt.RECEIPT_TYPE.value]), LockSecretExpiredFeeReceipt);
		mapping.set(ReceiptFactory.toKey([MosaicExpiredReceipt.RECEIPT_TYPE.value]), MosaicExpiredReceipt);
		mapping.set(ReceiptFactory.toKey([MosaicRentalFeeReceipt.RECEIPT_TYPE.value]), MosaicRentalFeeReceipt);
		mapping.set(ReceiptFactory.toKey([NamespaceExpiredReceipt.RECEIPT_TYPE.value]), NamespaceExpiredReceipt);
		mapping.set(ReceiptFactory.toKey([NamespaceDeletedReceipt.RECEIPT_TYPE.value]), NamespaceDeletedReceipt);
		mapping.set(ReceiptFactory.toKey([NamespaceRentalFeeReceipt.RECEIPT_TYPE.value]), NamespaceRentalFeeReceipt);
		const discriminator = ReceiptFactory.toKey([parent.type.value]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			harvest_fee_receipt: HarvestFeeReceipt,
			inflation_receipt: InflationReceipt,
			lock_hash_created_fee_receipt: LockHashCreatedFeeReceipt,
			lock_hash_completed_fee_receipt: LockHashCompletedFeeReceipt,
			lock_hash_expired_fee_receipt: LockHashExpiredFeeReceipt,
			lock_secret_created_fee_receipt: LockSecretCreatedFeeReceipt,
			lock_secret_completed_fee_receipt: LockSecretCompletedFeeReceipt,
			lock_secret_expired_fee_receipt: LockSecretExpiredFeeReceipt,
			mosaic_expired_receipt: MosaicExpiredReceipt,
			mosaic_rental_fee_receipt: MosaicRentalFeeReceipt,
			namespace_expired_receipt: NamespaceExpiredReceipt,
			namespace_deleted_receipt: NamespaceDeletedReceipt,
			namespace_rental_fee_receipt: NamespaceRentalFeeReceipt
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError(`unknown Receipt type ${entityName}`);

		return new mapping[entityName]();
	}
}
