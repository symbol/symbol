/* eslint-disable max-len, object-property-newline, no-underscore-dangle, no-use-before-define */

import BaseValue from '../BaseValue.js';
import ByteArray from '../ByteArray.js';
import BufferView from '../utils/BufferView.js';
import Writer from '../utils/Writer.js';
import * as arrayHelpers from '../utils/arrayHelpers.js';
import * as converter from '../utils/converter.js';
import { ripemdKeccak256 } from '../utils/transforms.js';

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

export class Timestamp extends BaseValue {
	static SIZE = 4;

	constructor(timestamp = 0) {
		super(Timestamp.SIZE, timestamp);
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Timestamp(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return new Timestamp(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}
}

export class Address extends ByteArray {
	static SIZE = 40;

	constructor(address = new Uint8Array(40)) {
		super(Address.SIZE, address);
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 40;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return new Address(new Uint8Array(byteArray.buffer, byteArray.byteOffset, 40));
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
	static TRANSFER = new TransactionType(257);

	static ACCOUNT_KEY_LINK = new TransactionType(2049);

	static MULTISIG_ACCOUNT_MODIFICATION = new TransactionType(4097);

	static MULTISIG_COSIGNATURE = new TransactionType(4098);

	static MULTISIG = new TransactionType(4100);

	static NAMESPACE_REGISTRATION = new TransactionType(8193);

	static MOSAIC_DEFINITION = new TransactionType(16385);

	static MOSAIC_SUPPLY_CHANGE = new TransactionType(16386);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			257, 2049, 4097, 4098, 4100, 8193, 16385, 16386
		];
		const keys = [
			'TRANSFER', 'ACCOUNT_KEY_LINK', 'MULTISIG_ACCOUNT_MODIFICATION', 'MULTISIG_COSIGNATURE', 'MULTISIG', 'NAMESPACE_REGISTRATION',
			'MOSAIC_DEFINITION', 'MOSAIC_SUPPLY_CHANGE'
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
		return 4;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}

	toString() {
		return `TransactionType.${TransactionType.valueToKey(this.value)}`;
	}
}

export class Transaction {
	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp'
	};

	constructor() {
		this._type = TransactionType.TRANSFER;
		this._version = 0;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);

		const instance = new Transaction();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += ')';
		return result;
	}
}

export class NonVerifiableTransaction {
	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp'
	};

	constructor() {
		this._type = TransactionType.TRANSFER;
		this._version = 0;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);

		const instance = new NonVerifiableTransaction();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += ')';
		return result;
	}
}

export class LinkAction {
	static LINK = new LinkAction(1);

	static UNLINK = new LinkAction(2);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			1, 2
		];
		const keys = [
			'LINK', 'UNLINK'
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
		return 4;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}

	toString() {
		return `LinkAction.${LinkAction.valueToKey(this.value)}`;
	}
}

export class AccountKeyLinkTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_KEY_LINK;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		linkAction: 'enum:LinkAction',
		remotePublicKey: 'pod:PublicKey'
	};

	constructor() {
		this._type = AccountKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._version = AccountKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkAction = LinkAction.LINK;
		this._remotePublicKey = new PublicKey();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._remotePublicKeySize = 32; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get remotePublicKey() {
		return this._remotePublicKey;
	}

	set remotePublicKey(value) {
		this._remotePublicKey = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.linkAction.size;
		size += 4;
		size += this.remotePublicKey.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);
		const remotePublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== remotePublicKeySize)
			throw RangeError(`Invalid value of reserved field (${remotePublicKeySize})`);
		const remotePublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(remotePublicKey.size);

		const instance = new AccountKeyLinkTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._linkAction = linkAction;
		instance._remotePublicKey = remotePublicKey;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._linkAction.serialize());
		buffer.write(converter.intToBytes(this._remotePublicKeySize, 4, false));
		buffer.write(this._remotePublicKey.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
		result += `remotePublicKey: ${this._remotePublicKey.toString()}, `;
		result += ')';
		return result;
	}
}

export class NonVerifiableAccountKeyLinkTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.ACCOUNT_KEY_LINK;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		linkAction: 'enum:LinkAction',
		remotePublicKey: 'pod:PublicKey'
	};

	constructor() {
		this._type = NonVerifiableAccountKeyLinkTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableAccountKeyLinkTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._linkAction = LinkAction.LINK;
		this._remotePublicKey = new PublicKey();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._remotePublicKeySize = 32; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get linkAction() {
		return this._linkAction;
	}

	set linkAction(value) {
		this._linkAction = value;
	}

	get remotePublicKey() {
		return this._remotePublicKey;
	}

	set remotePublicKey(value) {
		this._remotePublicKey = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += this.linkAction.size;
		size += 4;
		size += this.remotePublicKey.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const linkAction = LinkAction.deserialize(view.buffer);
		view.shiftRight(linkAction.size);
		const remotePublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== remotePublicKeySize)
			throw RangeError(`Invalid value of reserved field (${remotePublicKeySize})`);
		const remotePublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(remotePublicKey.size);

		const instance = new NonVerifiableAccountKeyLinkTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._linkAction = linkAction;
		instance._remotePublicKey = remotePublicKey;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(this._linkAction.serialize());
		buffer.write(converter.intToBytes(this._remotePublicKeySize, 4, false));
		buffer.write(this._remotePublicKey.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `linkAction: ${this._linkAction.toString()}, `;
		result += `remotePublicKey: ${this._remotePublicKey.toString()}, `;
		result += ')';
		return result;
	}
}

export class NamespaceId {
	static TYPE_HINTS = {
		name: 'bytes_array'
	};

	constructor() {
		this._name = new Uint8Array();
	}

	sort() { // eslint-disable-line class-methods-use-this
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
		size += this._name.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const nameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);

		const instance = new NamespaceId();
		instance._name = name;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._name.length, 4, false)); // bound: name_size
		buffer.write(this._name);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
		result += ')';
		return result;
	}
}

export class MosaicId {
	static TYPE_HINTS = {
		namespaceId: 'struct:NamespaceId',
		name: 'bytes_array'
	};

	constructor() {
		this._namespaceId = new NamespaceId();
		this._name = new Uint8Array();
	}

	sort() {
		this._namespaceId.sort();
	}

	get namespaceId() {
		return this._namespaceId;
	}

	set namespaceId(value) {
		this._namespaceId = value;
	}

	get name() {
		return this._name;
	}

	set name(value) {
		this._name = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.namespaceId.size;
		size += 4;
		size += this._name.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const namespaceId = NamespaceId.deserialize(view.buffer);
		view.shiftRight(namespaceId.size);
		const nameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);

		const instance = new MosaicId();
		instance._namespaceId = namespaceId;
		instance._name = name;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._namespaceId.serialize());
		buffer.write(converter.intToBytes(this._name.length, 4, false)); // bound: name_size
		buffer.write(this._name);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `namespaceId: ${this._namespaceId.toString()}, `;
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
		result += ')';
		return result;
	}
}

export class Mosaic {
	static TYPE_HINTS = {
		mosaicId: 'struct:MosaicId',
		amount: 'pod:Amount'
	};

	constructor() {
		this._mosaicId = new MosaicId();
		this._amount = new Amount();
	}

	sort() {
		this._mosaicId.sort();
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
		size += 4;
		size += this.mosaicId.size;
		size += this.amount.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicIdSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaicId = MosaicId.deserialize(view.window(mosaicIdSize));
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
		buffer.write(converter.intToBytes(this.mosaicId.size, 4, false)); // bound: mosaic_id_size
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

export class SizePrefixedMosaic {
	static TYPE_HINTS = {
		mosaic: 'struct:Mosaic'
	};

	constructor() {
		this._mosaic = new Mosaic();
	}

	sort() {
		this._mosaic.sort();
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
		size += this.mosaic.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const mosaicSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaic = Mosaic.deserialize(view.window(mosaicSize));
		view.shiftRight(mosaic.size);

		const instance = new SizePrefixedMosaic();
		instance._mosaic = mosaic;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.mosaic.size, 4, false)); // bound: mosaic_size
		buffer.write(this._mosaic.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `mosaic: ${this._mosaic.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicTransferFeeType {
	static ABSOLUTE = new MosaicTransferFeeType(1);

	static PERCENTILE = new MosaicTransferFeeType(2);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			1, 2
		];
		const keys = [
			'ABSOLUTE', 'PERCENTILE'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return MosaicTransferFeeType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 4;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}

	toString() {
		return `MosaicTransferFeeType.${MosaicTransferFeeType.valueToKey(this.value)}`;
	}
}

export class MosaicLevy {
	static TYPE_HINTS = {
		transferFeeType: 'enum:MosaicTransferFeeType',
		recipientAddress: 'pod:Address',
		mosaicId: 'struct:MosaicId',
		fee: 'pod:Amount'
	};

	constructor() {
		this._transferFeeType = MosaicTransferFeeType.ABSOLUTE;
		this._recipientAddress = new Address();
		this._mosaicId = new MosaicId();
		this._fee = new Amount();
		this._recipientAddressSize = 40; // reserved field
	}

	sort() {
		this._mosaicId.sort();
	}

	get transferFeeType() {
		return this._transferFeeType;
	}

	set transferFeeType(value) {
		this._transferFeeType = value;
	}

	get recipientAddress() {
		return this._recipientAddress;
	}

	set recipientAddress(value) {
		this._recipientAddress = value;
	}

	get mosaicId() {
		return this._mosaicId;
	}

	set mosaicId(value) {
		this._mosaicId = value;
	}

	get fee() {
		return this._fee;
	}

	set fee(value) {
		this._fee = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.transferFeeType.size;
		size += 4;
		size += this.recipientAddress.size;
		size += 4;
		size += this.mosaicId.size;
		size += this.fee.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const transferFeeType = MosaicTransferFeeType.deserialize(view.buffer);
		view.shiftRight(transferFeeType.size);
		const recipientAddressSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== recipientAddressSize)
			throw RangeError(`Invalid value of reserved field (${recipientAddressSize})`);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const mosaicIdSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaicId = MosaicId.deserialize(view.window(mosaicIdSize));
		view.shiftRight(mosaicId.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);

		const instance = new MosaicLevy();
		instance._transferFeeType = transferFeeType;
		instance._recipientAddress = recipientAddress;
		instance._mosaicId = mosaicId;
		instance._fee = fee;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._transferFeeType.serialize());
		buffer.write(converter.intToBytes(this._recipientAddressSize, 4, false));
		buffer.write(this._recipientAddress.serialize());
		buffer.write(converter.intToBytes(this.mosaicId.size, 4, false)); // bound: mosaic_id_size
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._fee.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `transferFeeType: ${this._transferFeeType.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicProperty {
	static TYPE_HINTS = {
		name: 'bytes_array',
		value: 'bytes_array'
	};

	constructor() {
		this._name = new Uint8Array();
		this._value = new Uint8Array();
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get name() {
		return this._name;
	}

	set name(value) {
		this._name = value;
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
		size += this._name.length;
		size += 4;
		size += this._value.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const nameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);
		const valueSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const value = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, valueSize);
		view.shiftRight(valueSize);

		const instance = new MosaicProperty();
		instance._name = name;
		instance._value = value;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._name.length, 4, false)); // bound: name_size
		buffer.write(this._name);
		buffer.write(converter.intToBytes(this._value.length, 4, false)); // bound: value_size
		buffer.write(this._value);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
		result += `value: hex(${converter.uint8ToHex(this._value)}), `;
		result += ')';
		return result;
	}
}

export class SizePrefixedMosaicProperty {
	static TYPE_HINTS = {
		property: 'struct:MosaicProperty'
	};

	constructor() {
		this._property = new MosaicProperty();
	}

	sort() {
		this._property.sort();
	}

	get property() {
		return this._property;
	}

	set property(value) {
		this._property = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += this.property.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const propertySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const property = MosaicProperty.deserialize(view.window(propertySize));
		view.shiftRight(property.size);

		const instance = new SizePrefixedMosaicProperty();
		instance._property = property;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.property.size, 4, false)); // bound: property_size
		buffer.write(this._property.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `property: ${this._property.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicDefinition {
	static TYPE_HINTS = {
		ownerPublicKey: 'pod:PublicKey',
		id: 'struct:MosaicId',
		description: 'bytes_array',
		properties: 'array[SizePrefixedMosaicProperty]',
		levy: 'struct:MosaicLevy'
	};

	constructor() {
		this._ownerPublicKey = new PublicKey();
		this._id = new MosaicId();
		this._description = new Uint8Array();
		this._properties = [];
		this._levy = null;
		this._ownerPublicKeySize = 32; // reserved field
	}

	sort() {
		this._id.sort();
		if (0 !== this.levySizeComputed)
			this._levy.sort();
	}

	get ownerPublicKey() {
		return this._ownerPublicKey;
	}

	set ownerPublicKey(value) {
		this._ownerPublicKey = value;
	}

	get id() {
		return this._id;
	}

	set id(value) {
		this._id = value;
	}

	get description() {
		return this._description;
	}

	set description(value) {
		this._description = value;
	}

	get properties() {
		return this._properties;
	}

	set properties(value) {
		this._properties = value;
	}

	get levy() {
		return this._levy;
	}

	set levy(value) {
		this._levy = value;
	}

	get levySizeComputed() {
		return this.levy ? this.levy.size + 0 : 0;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += this.ownerPublicKey.size;
		size += 4;
		size += this.id.size;
		size += 4;
		size += this._description.length;
		size += 4;
		size += arrayHelpers.size(this.properties);
		size += 4;
		if (0 !== this.levySizeComputed)
			size += this.levy.size;

		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const ownerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== ownerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${ownerPublicKeySize})`);
		const ownerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(ownerPublicKey.size);
		const idSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const id = MosaicId.deserialize(view.window(idSize));
		view.shiftRight(id.size);
		const descriptionSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const description = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, descriptionSize);
		view.shiftRight(descriptionSize);
		const propertiesCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const properties = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMosaicProperty, propertiesCount);
		view.shiftRight(arrayHelpers.size(properties));
		const levySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let levy = null;
		if (0 !== levySize) {
			levy = MosaicLevy.deserialize(view.buffer);
			view.shiftRight(levy.size);
		}

		const instance = new MosaicDefinition();
		instance._ownerPublicKey = ownerPublicKey;
		instance._id = id;
		instance._description = description;
		instance._properties = properties;
		instance._levy = levy;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this._ownerPublicKeySize, 4, false));
		buffer.write(this._ownerPublicKey.serialize());
		buffer.write(converter.intToBytes(this.id.size, 4, false)); // bound: id_size
		buffer.write(this._id.serialize());
		buffer.write(converter.intToBytes(this._description.length, 4, false)); // bound: description_size
		buffer.write(this._description);
		buffer.write(converter.intToBytes(this._properties.length, 4, false)); // bound: properties_count
		arrayHelpers.writeArray(buffer, this._properties);
		buffer.write(converter.intToBytes(this.levySizeComputed, 4, false));
		if (0 !== this.levySizeComputed)
			buffer.write(this._levy.serialize());

		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `ownerPublicKey: ${this._ownerPublicKey.toString()}, `;
		result += `id: ${this._id.toString()}, `;
		result += `description: hex(${converter.uint8ToHex(this._description)}), `;
		result += `properties: [${this._properties.map(e => e.toString()).join(',')}], `;
		if (0 !== this.levySizeComputed)
			result += `levy: ${this._levy.toString()}, `;

		result += ')';
		return result;
	}
}

export class MosaicDefinitionTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_DEFINITION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicDefinition: 'struct:MosaicDefinition',
		rentalFeeSink: 'pod:Address',
		rentalFee: 'pod:Amount'
	};

	constructor() {
		this._type = MosaicDefinitionTransactionV1.TRANSACTION_TYPE;
		this._version = MosaicDefinitionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicDefinition = new MosaicDefinition();
		this._rentalFeeSink = new Address();
		this._rentalFee = new Amount();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._rentalFeeSinkSize = 40; // reserved field
	}

	sort() {
		this._mosaicDefinition.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get mosaicDefinition() {
		return this._mosaicDefinition;
	}

	set mosaicDefinition(value) {
		this._mosaicDefinition = value;
	}

	get rentalFeeSink() {
		return this._rentalFeeSink;
	}

	set rentalFeeSink(value) {
		this._rentalFeeSink = value;
	}

	get rentalFee() {
		return this._rentalFee;
	}

	set rentalFee(value) {
		this._rentalFee = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.mosaicDefinition.size;
		size += 4;
		size += this.rentalFeeSink.size;
		size += this.rentalFee.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicDefinitionSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaicDefinition = MosaicDefinition.deserialize(view.window(mosaicDefinitionSize));
		view.shiftRight(mosaicDefinition.size);
		const rentalFeeSinkSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== rentalFeeSinkSize)
			throw RangeError(`Invalid value of reserved field (${rentalFeeSinkSize})`);
		const rentalFeeSink = Address.deserialize(view.buffer);
		view.shiftRight(rentalFeeSink.size);
		const rentalFee = Amount.deserialize(view.buffer);
		view.shiftRight(rentalFee.size);

		const instance = new MosaicDefinitionTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicDefinition = mosaicDefinition;
		instance._rentalFeeSink = rentalFeeSink;
		instance._rentalFee = rentalFee;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this.mosaicDefinition.size, 4, false)); // bound: mosaic_definition_size
		buffer.write(this._mosaicDefinition.serialize());
		buffer.write(converter.intToBytes(this._rentalFeeSinkSize, 4, false));
		buffer.write(this._rentalFeeSink.serialize());
		buffer.write(this._rentalFee.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicDefinition: ${this._mosaicDefinition.toString()}, `;
		result += `rentalFeeSink: ${this._rentalFeeSink.toString()}, `;
		result += `rentalFee: ${this._rentalFee.toString()}, `;
		result += ')';
		return result;
	}
}

export class NonVerifiableMosaicDefinitionTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_DEFINITION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicDefinition: 'struct:MosaicDefinition',
		rentalFeeSink: 'pod:Address',
		rentalFee: 'pod:Amount'
	};

	constructor() {
		this._type = NonVerifiableMosaicDefinitionTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableMosaicDefinitionTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicDefinition = new MosaicDefinition();
		this._rentalFeeSink = new Address();
		this._rentalFee = new Amount();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._rentalFeeSinkSize = 40; // reserved field
	}

	sort() {
		this._mosaicDefinition.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get mosaicDefinition() {
		return this._mosaicDefinition;
	}

	set mosaicDefinition(value) {
		this._mosaicDefinition = value;
	}

	get rentalFeeSink() {
		return this._rentalFeeSink;
	}

	set rentalFeeSink(value) {
		this._rentalFeeSink = value;
	}

	get rentalFee() {
		return this._rentalFee;
	}

	set rentalFee(value) {
		this._rentalFee = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.mosaicDefinition.size;
		size += 4;
		size += this.rentalFeeSink.size;
		size += this.rentalFee.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicDefinitionSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaicDefinition = MosaicDefinition.deserialize(view.window(mosaicDefinitionSize));
		view.shiftRight(mosaicDefinition.size);
		const rentalFeeSinkSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== rentalFeeSinkSize)
			throw RangeError(`Invalid value of reserved field (${rentalFeeSinkSize})`);
		const rentalFeeSink = Address.deserialize(view.buffer);
		view.shiftRight(rentalFeeSink.size);
		const rentalFee = Amount.deserialize(view.buffer);
		view.shiftRight(rentalFee.size);

		const instance = new NonVerifiableMosaicDefinitionTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicDefinition = mosaicDefinition;
		instance._rentalFeeSink = rentalFeeSink;
		instance._rentalFee = rentalFee;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this.mosaicDefinition.size, 4, false)); // bound: mosaic_definition_size
		buffer.write(this._mosaicDefinition.serialize());
		buffer.write(converter.intToBytes(this._rentalFeeSinkSize, 4, false));
		buffer.write(this._rentalFeeSink.serialize());
		buffer.write(this._rentalFee.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicDefinition: ${this._mosaicDefinition.toString()}, `;
		result += `rentalFeeSink: ${this._rentalFeeSink.toString()}, `;
		result += `rentalFee: ${this._rentalFee.toString()}, `;
		result += ')';
		return result;
	}
}

export class MosaicSupplyChangeAction {
	static INCREASE = new MosaicSupplyChangeAction(1);

	static DECREASE = new MosaicSupplyChangeAction(2);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			1, 2
		];
		const keys = [
			'INCREASE', 'DECREASE'
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
		return 4;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}

	toString() {
		return `MosaicSupplyChangeAction.${MosaicSupplyChangeAction.valueToKey(this.value)}`;
	}
}

export class MosaicSupplyChangeTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_SUPPLY_CHANGE;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicId: 'struct:MosaicId',
		action: 'enum:MosaicSupplyChangeAction',
		delta: 'pod:Amount'
	};

	constructor() {
		this._type = MosaicSupplyChangeTransactionV1.TRANSACTION_TYPE;
		this._version = MosaicSupplyChangeTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicId = new MosaicId();
		this._action = MosaicSupplyChangeAction.INCREASE;
		this._delta = new Amount();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
	}

	sort() {
		this._mosaicId.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get action() {
		return this._action;
	}

	set action(value) {
		this._action = value;
	}

	get delta() {
		return this._delta;
	}

	set delta(value) {
		this._delta = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.mosaicId.size;
		size += this.action.size;
		size += this.delta.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicIdSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaicId = MosaicId.deserialize(view.window(mosaicIdSize));
		view.shiftRight(mosaicId.size);
		const action = MosaicSupplyChangeAction.deserialize(view.buffer);
		view.shiftRight(action.size);
		const delta = Amount.deserialize(view.buffer);
		view.shiftRight(delta.size);

		const instance = new MosaicSupplyChangeTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicId = mosaicId;
		instance._action = action;
		instance._delta = delta;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this.mosaicId.size, 4, false)); // bound: mosaic_id_size
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._action.serialize());
		buffer.write(this._delta.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `action: ${this._action.toString()}, `;
		result += `delta: ${this._delta.toString()}, `;
		result += ')';
		return result;
	}
}

export class NonVerifiableMosaicSupplyChangeTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MOSAIC_SUPPLY_CHANGE;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		mosaicId: 'struct:MosaicId',
		action: 'enum:MosaicSupplyChangeAction',
		delta: 'pod:Amount'
	};

	constructor() {
		this._type = NonVerifiableMosaicSupplyChangeTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableMosaicSupplyChangeTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._mosaicId = new MosaicId();
		this._action = MosaicSupplyChangeAction.INCREASE;
		this._delta = new Amount();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
	}

	sort() {
		this._mosaicId.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get action() {
		return this._action;
	}

	set action(value) {
		this._action = value;
	}

	get delta() {
		return this._delta;
	}

	set delta(value) {
		this._delta = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.mosaicId.size;
		size += this.action.size;
		size += this.delta.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const mosaicIdSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const mosaicId = MosaicId.deserialize(view.window(mosaicIdSize));
		view.shiftRight(mosaicId.size);
		const action = MosaicSupplyChangeAction.deserialize(view.buffer);
		view.shiftRight(action.size);
		const delta = Amount.deserialize(view.buffer);
		view.shiftRight(delta.size);

		const instance = new NonVerifiableMosaicSupplyChangeTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._mosaicId = mosaicId;
		instance._action = action;
		instance._delta = delta;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this.mosaicId.size, 4, false)); // bound: mosaic_id_size
		buffer.write(this._mosaicId.serialize());
		buffer.write(this._action.serialize());
		buffer.write(this._delta.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `mosaicId: ${this._mosaicId.toString()}, `;
		result += `action: ${this._action.toString()}, `;
		result += `delta: ${this._delta.toString()}, `;
		result += ')';
		return result;
	}
}

export class MultisigAccountModificationType {
	static ADD_COSIGNATORY = new MultisigAccountModificationType(1);

	static DELETE_COSIGNATORY = new MultisigAccountModificationType(2);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			1, 2
		];
		const keys = [
			'ADD_COSIGNATORY', 'DELETE_COSIGNATORY'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return MultisigAccountModificationType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 4;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}

	toString() {
		return `MultisigAccountModificationType.${MultisigAccountModificationType.valueToKey(this.value)}`;
	}
}

export class MultisigAccountModification {
	static TYPE_HINTS = {
		modificationType: 'enum:MultisigAccountModificationType',
		cosignatoryPublicKey: 'pod:PublicKey'
	};

	constructor() {
		this._modificationType = MultisigAccountModificationType.ADD_COSIGNATORY;
		this._cosignatoryPublicKey = new PublicKey();
		this._cosignatoryPublicKeySize = 32; // reserved field
	}

	comparer() {
		return [
			this.modificationType,
			ripemdKeccak256(this.cosignatoryPublicKey.bytes)
		];
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get modificationType() {
		return this._modificationType;
	}

	set modificationType(value) {
		this._modificationType = value;
	}

	get cosignatoryPublicKey() {
		return this._cosignatoryPublicKey;
	}

	set cosignatoryPublicKey(value) {
		this._cosignatoryPublicKey = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.modificationType.size;
		size += 4;
		size += this.cosignatoryPublicKey.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const modificationType = MultisigAccountModificationType.deserialize(view.buffer);
		view.shiftRight(modificationType.size);
		const cosignatoryPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== cosignatoryPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${cosignatoryPublicKeySize})`);
		const cosignatoryPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(cosignatoryPublicKey.size);

		const instance = new MultisigAccountModification();
		instance._modificationType = modificationType;
		instance._cosignatoryPublicKey = cosignatoryPublicKey;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._modificationType.serialize());
		buffer.write(converter.intToBytes(this._cosignatoryPublicKeySize, 4, false));
		buffer.write(this._cosignatoryPublicKey.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `modificationType: ${this._modificationType.toString()}, `;
		result += `cosignatoryPublicKey: ${this._cosignatoryPublicKey.toString()}, `;
		result += ')';
		return result;
	}
}

export class SizePrefixedMultisigAccountModification {
	static TYPE_HINTS = {
		modification: 'struct:MultisigAccountModification'
	};

	constructor() {
		this._modification = new MultisigAccountModification();
	}

	sort() {
		this._modification.sort();
	}

	get modification() {
		return this._modification;
	}

	set modification(value) {
		this._modification = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += this.modification.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const modificationSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const modification = MultisigAccountModification.deserialize(view.window(modificationSize));
		view.shiftRight(modification.size);

		const instance = new SizePrefixedMultisigAccountModification();
		instance._modification = modification;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.modification.size, 4, false)); // bound: modification_size
		buffer.write(this._modification.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `modification: ${this._modification.toString()}, `;
		result += ')';
		return result;
	}
}

export class MultisigAccountModificationTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_ACCOUNT_MODIFICATION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		modifications: 'array[SizePrefixedMultisigAccountModification]'
	};

	constructor() {
		this._type = MultisigAccountModificationTransactionV1.TRANSACTION_TYPE;
		this._version = MultisigAccountModificationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._modifications = [];
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
	}

	sort() {
		this._modifications = this._modifications.sort((lhs, rhs) => arrayHelpers.deepCompare(
			(lhs.modification.comparer ? lhs.modification.comparer() : lhs.modification.value),
			(rhs.modification.comparer ? rhs.modification.comparer() : rhs.modification.value)
		));
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get modifications() {
		return this._modifications;
	}

	set modifications(value) {
		this._modifications = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += arrayHelpers.size(this.modifications);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const modificationsCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const modifications = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMultisigAccountModification, modificationsCount, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		view.shiftRight(arrayHelpers.size(modifications));

		const instance = new MultisigAccountModificationTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._modifications = modifications;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._modifications.length, 4, false)); // bound: modifications_count
		arrayHelpers.writeArray(buffer, this._modifications, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `modifications: [${this._modifications.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class NonVerifiableMultisigAccountModificationTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_ACCOUNT_MODIFICATION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		modifications: 'array[SizePrefixedMultisigAccountModification]'
	};

	constructor() {
		this._type = NonVerifiableMultisigAccountModificationTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableMultisigAccountModificationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._modifications = [];
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
	}

	sort() {
		this._modifications = this._modifications.sort((lhs, rhs) => arrayHelpers.deepCompare(
			(lhs.modification.comparer ? lhs.modification.comparer() : lhs.modification.value),
			(rhs.modification.comparer ? rhs.modification.comparer() : rhs.modification.value)
		));
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get modifications() {
		return this._modifications;
	}

	set modifications(value) {
		this._modifications = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += arrayHelpers.size(this.modifications);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const modificationsCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const modifications = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMultisigAccountModification, modificationsCount, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		view.shiftRight(arrayHelpers.size(modifications));

		const instance = new NonVerifiableMultisigAccountModificationTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._modifications = modifications;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._modifications.length, 4, false)); // bound: modifications_count
		arrayHelpers.writeArray(buffer, this._modifications, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `modifications: [${this._modifications.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class MultisigAccountModificationTransactionV2 {
	static TRANSACTION_VERSION = 2;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_ACCOUNT_MODIFICATION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		modifications: 'array[SizePrefixedMultisigAccountModification]'
	};

	constructor() {
		this._type = MultisigAccountModificationTransactionV2.TRANSACTION_TYPE;
		this._version = MultisigAccountModificationTransactionV2.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._modifications = [];
		this._minApprovalDelta = 0;
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._minApprovalDeltaSize = 4; // reserved field
	}

	sort() {
		this._modifications = this._modifications.sort((lhs, rhs) => arrayHelpers.deepCompare(
			(lhs.modification.comparer ? lhs.modification.comparer() : lhs.modification.value),
			(rhs.modification.comparer ? rhs.modification.comparer() : rhs.modification.value)
		));
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get modifications() {
		return this._modifications;
	}

	set modifications(value) {
		this._modifications = value;
	}

	get minApprovalDelta() {
		return this._minApprovalDelta;
	}

	set minApprovalDelta(value) {
		this._minApprovalDelta = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += arrayHelpers.size(this.modifications);
		size += 4;
		size += 4;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const modificationsCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const modifications = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMultisigAccountModification, modificationsCount, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		view.shiftRight(arrayHelpers.size(modifications));
		const minApprovalDeltaSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (4 !== minApprovalDeltaSize)
			throw RangeError(`Invalid value of reserved field (${minApprovalDeltaSize})`);
		const minApprovalDelta = converter.bytesToIntUnaligned(view.buffer, 4, true);
		view.shiftRight(4);

		const instance = new MultisigAccountModificationTransactionV2();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._modifications = modifications;
		instance._minApprovalDelta = minApprovalDelta;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._modifications.length, 4, false)); // bound: modifications_count
		arrayHelpers.writeArray(buffer, this._modifications, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		buffer.write(converter.intToBytes(this._minApprovalDeltaSize, 4, false));
		buffer.write(converter.intToBytes(this._minApprovalDelta, 4, true));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `modifications: [${this._modifications.map(e => e.toString()).join(',')}], `;
		result += `minApprovalDelta: ${'0x'.concat(this._minApprovalDelta.toString(16))}, `;
		result += ')';
		return result;
	}
}

export class NonVerifiableMultisigAccountModificationTransactionV2 {
	static TRANSACTION_VERSION = 2;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_ACCOUNT_MODIFICATION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		modifications: 'array[SizePrefixedMultisigAccountModification]'
	};

	constructor() {
		this._type = NonVerifiableMultisigAccountModificationTransactionV2.TRANSACTION_TYPE;
		this._version = NonVerifiableMultisigAccountModificationTransactionV2.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._modifications = [];
		this._minApprovalDelta = 0;
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._minApprovalDeltaSize = 4; // reserved field
	}

	sort() {
		this._modifications = this._modifications.sort((lhs, rhs) => arrayHelpers.deepCompare(
			(lhs.modification.comparer ? lhs.modification.comparer() : lhs.modification.value),
			(rhs.modification.comparer ? rhs.modification.comparer() : rhs.modification.value)
		));
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get modifications() {
		return this._modifications;
	}

	set modifications(value) {
		this._modifications = value;
	}

	get minApprovalDelta() {
		return this._minApprovalDelta;
	}

	set minApprovalDelta(value) {
		this._minApprovalDelta = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += arrayHelpers.size(this.modifications);
		size += 4;
		size += 4;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const modificationsCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const modifications = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMultisigAccountModification, modificationsCount, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		view.shiftRight(arrayHelpers.size(modifications));
		const minApprovalDeltaSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (4 !== minApprovalDeltaSize)
			throw RangeError(`Invalid value of reserved field (${minApprovalDeltaSize})`);
		const minApprovalDelta = converter.bytesToIntUnaligned(view.buffer, 4, true);
		view.shiftRight(4);

		const instance = new NonVerifiableMultisigAccountModificationTransactionV2();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._modifications = modifications;
		instance._minApprovalDelta = minApprovalDelta;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._modifications.length, 4, false)); // bound: modifications_count
		arrayHelpers.writeArray(buffer, this._modifications, e => ((e.modification.comparer ? e.modification.comparer() : e.modification.value)));
		buffer.write(converter.intToBytes(this._minApprovalDeltaSize, 4, false));
		buffer.write(converter.intToBytes(this._minApprovalDelta, 4, true));
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `modifications: [${this._modifications.map(e => e.toString()).join(',')}], `;
		result += `minApprovalDelta: ${'0x'.concat(this._minApprovalDelta.toString(16))}, `;
		result += ')';
		return result;
	}
}

export class CosignatureV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG_COSIGNATURE;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		multisigTransactionHash: 'pod:Hash256',
		multisigAccountAddress: 'pod:Address'
	};

	constructor() {
		this._type = CosignatureV1.TRANSACTION_TYPE;
		this._version = CosignatureV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._multisigTransactionHash = new Hash256();
		this._multisigAccountAddress = new Address();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._multisigTransactionHashOuterSize = 36; // reserved field
		this._multisigTransactionHashSize = 32; // reserved field
		this._multisigAccountAddressSize = 40; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get multisigTransactionHash() {
		return this._multisigTransactionHash;
	}

	set multisigTransactionHash(value) {
		this._multisigTransactionHash = value;
	}

	get multisigAccountAddress() {
		return this._multisigAccountAddress;
	}

	set multisigAccountAddress(value) {
		this._multisigAccountAddress = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += 4;
		size += this.multisigTransactionHash.size;
		size += 4;
		size += this.multisigAccountAddress.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const multisigTransactionHashOuterSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (36 !== multisigTransactionHashOuterSize)
			throw RangeError(`Invalid value of reserved field (${multisigTransactionHashOuterSize})`);
		const multisigTransactionHashSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== multisigTransactionHashSize)
			throw RangeError(`Invalid value of reserved field (${multisigTransactionHashSize})`);
		const multisigTransactionHash = Hash256.deserialize(view.buffer);
		view.shiftRight(multisigTransactionHash.size);
		const multisigAccountAddressSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== multisigAccountAddressSize)
			throw RangeError(`Invalid value of reserved field (${multisigAccountAddressSize})`);
		const multisigAccountAddress = Address.deserialize(view.buffer);
		view.shiftRight(multisigAccountAddress.size);

		const instance = new CosignatureV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._multisigTransactionHash = multisigTransactionHash;
		instance._multisigAccountAddress = multisigAccountAddress;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._multisigTransactionHashOuterSize, 4, false));
		buffer.write(converter.intToBytes(this._multisigTransactionHashSize, 4, false));
		buffer.write(this._multisigTransactionHash.serialize());
		buffer.write(converter.intToBytes(this._multisigAccountAddressSize, 4, false));
		buffer.write(this._multisigAccountAddress.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `multisigTransactionHash: ${this._multisigTransactionHash.toString()}, `;
		result += `multisigAccountAddress: ${this._multisigAccountAddress.toString()}, `;
		result += ')';
		return result;
	}
}

export class SizePrefixedCosignatureV1 {
	static TYPE_HINTS = {
		cosignature: 'struct:CosignatureV1'
	};

	constructor() {
		this._cosignature = new CosignatureV1();
	}

	sort() {
		this._cosignature.sort();
	}

	get cosignature() {
		return this._cosignature;
	}

	set cosignature(value) {
		this._cosignature = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += 4;
		size += this.cosignature.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const cosignatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const cosignature = CosignatureV1.deserialize(view.window(cosignatureSize));
		view.shiftRight(cosignature.size);

		const instance = new SizePrefixedCosignatureV1();
		instance._cosignature = cosignature;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(converter.intToBytes(this.cosignature.size, 4, false)); // bound: cosignature_size
		buffer.write(this._cosignature.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `cosignature: ${this._cosignature.toString()}, `;
		result += ')';
		return result;
	}
}

export class MultisigTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		innerTransaction: 'struct:NonVerifiableTransaction',
		cosignatures: 'array[SizePrefixedCosignatureV1]'
	};

	constructor() {
		this._type = MultisigTransactionV1.TRANSACTION_TYPE;
		this._version = MultisigTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._innerTransaction = new NonVerifiableTransaction();
		this._cosignatures = [];
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
	}

	sort() {
		this._innerTransaction.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get innerTransaction() {
		return this._innerTransaction;
	}

	set innerTransaction(value) {
		this._innerTransaction = value;
	}

	get cosignatures() {
		return this._cosignatures;
	}

	set cosignatures(value) {
		this._cosignatures = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.innerTransaction.size;
		size += 4;
		size += arrayHelpers.size(this.cosignatures);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const innerTransactionSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const innerTransaction = NonVerifiableTransactionFactory.deserialize(view.window(innerTransactionSize));
		view.shiftRight(innerTransaction.size);
		const cosignaturesCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const cosignatures = arrayHelpers.readArrayCount(view.buffer, SizePrefixedCosignatureV1, cosignaturesCount);
		view.shiftRight(arrayHelpers.size(cosignatures));

		const instance = new MultisigTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._innerTransaction = innerTransaction;
		instance._cosignatures = cosignatures;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this.innerTransaction.size, 4, false)); // bound: inner_transaction_size
		buffer.write(this._innerTransaction.serialize());
		buffer.write(converter.intToBytes(this._cosignatures.length, 4, false)); // bound: cosignatures_count
		arrayHelpers.writeArray(buffer, this._cosignatures);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `innerTransaction: ${this._innerTransaction.toString()}, `;
		result += `cosignatures: [${this._cosignatures.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class NonVerifiableMultisigTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.MULTISIG;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		innerTransaction: 'struct:NonVerifiableTransaction'
	};

	constructor() {
		this._type = NonVerifiableMultisigTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableMultisigTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._innerTransaction = new NonVerifiableTransaction();
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
	}

	sort() {
		this._innerTransaction.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get innerTransaction() {
		return this._innerTransaction;
	}

	set innerTransaction(value) {
		this._innerTransaction = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.innerTransaction.size;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const innerTransactionSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		// marking sizeof field
		const innerTransaction = NonVerifiableTransactionFactory.deserialize(view.window(innerTransactionSize));
		view.shiftRight(innerTransaction.size);

		const instance = new NonVerifiableMultisigTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._innerTransaction = innerTransaction;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this.innerTransaction.size, 4, false)); // bound: inner_transaction_size
		buffer.write(this._innerTransaction.serialize());
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `innerTransaction: ${this._innerTransaction.toString()}, `;
		result += ')';
		return result;
	}
}

export class NamespaceRegistrationTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NAMESPACE_REGISTRATION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		rentalFeeSink: 'pod:Address',
		rentalFee: 'pod:Amount',
		name: 'bytes_array',
		parentName: 'bytes_array'
	};

	constructor() {
		this._type = NamespaceRegistrationTransactionV1.TRANSACTION_TYPE;
		this._version = NamespaceRegistrationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._rentalFeeSink = new Address();
		this._rentalFee = new Amount();
		this._name = new Uint8Array();
		this._parentName = null;
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._rentalFeeSinkSize = 40; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get rentalFeeSink() {
		return this._rentalFeeSink;
	}

	set rentalFeeSink(value) {
		this._rentalFeeSink = value;
	}

	get rentalFee() {
		return this._rentalFee;
	}

	set rentalFee(value) {
		this._rentalFee = value;
	}

	get name() {
		return this._name;
	}

	set name(value) {
		this._name = value;
	}

	get parentName() {
		return this._parentName;
	}

	set parentName(value) {
		this._parentName = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.rentalFeeSink.size;
		size += this.rentalFee.size;
		size += 4;
		size += this._name.length;
		size += 4;
		if (this.parentName)
			size += this._parentName.length;

		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const rentalFeeSinkSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== rentalFeeSinkSize)
			throw RangeError(`Invalid value of reserved field (${rentalFeeSinkSize})`);
		const rentalFeeSink = Address.deserialize(view.buffer);
		view.shiftRight(rentalFeeSink.size);
		const rentalFee = Amount.deserialize(view.buffer);
		view.shiftRight(rentalFee.size);
		const nameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);
		const parentNameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let parentName = null;
		if (4294967295 !== parentNameSize) {
			parentName = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, parentNameSize);
			view.shiftRight(parentNameSize);
		}

		const instance = new NamespaceRegistrationTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._rentalFeeSink = rentalFeeSink;
		instance._rentalFee = rentalFee;
		instance._name = name;
		instance._parentName = parentName;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._rentalFeeSinkSize, 4, false));
		buffer.write(this._rentalFeeSink.serialize());
		buffer.write(this._rentalFee.serialize());
		buffer.write(converter.intToBytes(this._name.length, 4, false)); // bound: name_size
		buffer.write(this._name);
		buffer.write(converter.intToBytes((this._parentName ? this._parentName.length : 4294967295), 4, false)); // bound: parent_name_size
		if (this.parentName)
			buffer.write(this._parentName);

		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `rentalFeeSink: ${this._rentalFeeSink.toString()}, `;
		result += `rentalFee: ${this._rentalFee.toString()}, `;
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
		if (this.parentName)
			result += `parentName: hex(${converter.uint8ToHex(this._parentName)}), `;

		result += ')';
		return result;
	}
}

export class NonVerifiableNamespaceRegistrationTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.NAMESPACE_REGISTRATION;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		rentalFeeSink: 'pod:Address',
		rentalFee: 'pod:Amount',
		name: 'bytes_array',
		parentName: 'bytes_array'
	};

	constructor() {
		this._type = NonVerifiableNamespaceRegistrationTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableNamespaceRegistrationTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._rentalFeeSink = new Address();
		this._rentalFee = new Amount();
		this._name = new Uint8Array();
		this._parentName = null;
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._rentalFeeSinkSize = 40; // reserved field
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get rentalFeeSink() {
		return this._rentalFeeSink;
	}

	set rentalFeeSink(value) {
		this._rentalFeeSink = value;
	}

	get rentalFee() {
		return this._rentalFee;
	}

	set rentalFee(value) {
		this._rentalFee = value;
	}

	get name() {
		return this._name;
	}

	set name(value) {
		this._name = value;
	}

	get parentName() {
		return this._parentName;
	}

	set parentName(value) {
		this._parentName = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.rentalFeeSink.size;
		size += this.rentalFee.size;
		size += 4;
		size += this._name.length;
		size += 4;
		if (this.parentName)
			size += this._parentName.length;

		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const rentalFeeSinkSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== rentalFeeSinkSize)
			throw RangeError(`Invalid value of reserved field (${rentalFeeSinkSize})`);
		const rentalFeeSink = Address.deserialize(view.buffer);
		view.shiftRight(rentalFeeSink.size);
		const rentalFee = Amount.deserialize(view.buffer);
		view.shiftRight(rentalFee.size);
		const nameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const name = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, nameSize);
		view.shiftRight(nameSize);
		const parentNameSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let parentName = null;
		if (4294967295 !== parentNameSize) {
			parentName = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, parentNameSize);
			view.shiftRight(parentNameSize);
		}

		const instance = new NonVerifiableNamespaceRegistrationTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._rentalFeeSink = rentalFeeSink;
		instance._rentalFee = rentalFee;
		instance._name = name;
		instance._parentName = parentName;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._rentalFeeSinkSize, 4, false));
		buffer.write(this._rentalFeeSink.serialize());
		buffer.write(this._rentalFee.serialize());
		buffer.write(converter.intToBytes(this._name.length, 4, false)); // bound: name_size
		buffer.write(this._name);
		buffer.write(converter.intToBytes((this._parentName ? this._parentName.length : 4294967295), 4, false)); // bound: parent_name_size
		if (this.parentName)
			buffer.write(this._parentName);

		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `rentalFeeSink: ${this._rentalFeeSink.toString()}, `;
		result += `rentalFee: ${this._rentalFee.toString()}, `;
		result += `name: hex(${converter.uint8ToHex(this._name)}), `;
		if (this.parentName)
			result += `parentName: hex(${converter.uint8ToHex(this._parentName)}), `;

		result += ')';
		return result;
	}
}

export class MessageType {
	static PLAIN = new MessageType(1);

	static ENCRYPTED = new MessageType(2);

	constructor(value) {
		this.value = value;
	}

	static valueToKey(value) {
		const values = [
			1, 2
		];
		const keys = [
			'PLAIN', 'ENCRYPTED'
		];

		const index = values.indexOf(value);
		if (-1 === index)
			throw RangeError(`invalid enum value ${value}`);

		return keys[index];
	}

	static fromValue(value) {
		return MessageType[this.valueToKey(value)];
	}

	get size() { // eslint-disable-line class-methods-use-this
		return 4;
	}

	static deserialize(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToIntUnaligned(byteArray, 4, false));
	}

	static deserializeAligned(payload) {
		const byteArray = payload;
		return this.fromValue(converter.bytesToInt(byteArray, 4, false));
	}

	serialize() {
		return converter.intToBytes(this.value, 4, false);
	}

	toString() {
		return `MessageType.${MessageType.valueToKey(this.value)}`;
	}
}

export class Message {
	static TYPE_HINTS = {
		messageType: 'enum:MessageType',
		message: 'bytes_array'
	};

	constructor() {
		this._messageType = MessageType.PLAIN;
		this._message = new Uint8Array();
	}

	sort() { // eslint-disable-line class-methods-use-this
	}

	get messageType() {
		return this._messageType;
	}

	set messageType(value) {
		this._messageType = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.messageType.size;
		size += 4;
		size += this._message.length;
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const messageType = MessageType.deserialize(view.buffer);
		view.shiftRight(messageType.size);
		const messageSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const message = new Uint8Array(view.buffer.buffer, view.buffer.byteOffset, messageSize);
		view.shiftRight(messageSize);

		const instance = new Message();
		instance._messageType = messageType;
		instance._message = message;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._messageType.serialize());
		buffer.write(converter.intToBytes(this._message.length, 4, false)); // bound: message_size
		buffer.write(this._message);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `messageType: ${this._messageType.toString()}, `;
		result += `message: hex(${converter.uint8ToHex(this._message)}), `;
		result += ')';
		return result;
	}
}

export class TransferTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.TRANSFER;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:Address',
		amount: 'pod:Amount',
		message: 'struct:Message'
	};

	constructor() {
		this._type = TransferTransactionV1.TRANSACTION_TYPE;
		this._version = TransferTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new Address();
		this._amount = new Amount();
		this._message = null;
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._recipientAddressSize = 40; // reserved field
	}

	sort() {
		if (0 !== this.messageEnvelopeSizeComputed)
			this._message.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get amount() {
		return this._amount;
	}

	set amount(value) {
		this._amount = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get messageEnvelopeSizeComputed() {
		return this.message ? this.message.size + 0 : 0;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.recipientAddress.size;
		size += this.amount.size;
		size += 4;
		if (0 !== this.messageEnvelopeSizeComputed)
			size += this.message.size;

		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddressSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== recipientAddressSize)
			throw RangeError(`Invalid value of reserved field (${recipientAddressSize})`);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const amount = Amount.deserialize(view.buffer);
		view.shiftRight(amount.size);
		const messageEnvelopeSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let message = null;
		if (0 !== messageEnvelopeSize) {
			message = Message.deserialize(view.buffer);
			view.shiftRight(message.size);
		}

		const instance = new TransferTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._amount = amount;
		instance._message = message;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._recipientAddressSize, 4, false));
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._amount.serialize());
		buffer.write(converter.intToBytes(this.messageEnvelopeSizeComputed, 4, false));
		if (0 !== this.messageEnvelopeSizeComputed)
			buffer.write(this._message.serialize());

		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `amount: ${this._amount.toString()}, `;
		if (0 !== this.messageEnvelopeSizeComputed)
			result += `message: ${this._message.toString()}, `;

		result += ')';
		return result;
	}
}

export class NonVerifiableTransferTransactionV1 {
	static TRANSACTION_VERSION = 1;

	static TRANSACTION_TYPE = TransactionType.TRANSFER;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:Address',
		amount: 'pod:Amount',
		message: 'struct:Message'
	};

	constructor() {
		this._type = NonVerifiableTransferTransactionV1.TRANSACTION_TYPE;
		this._version = NonVerifiableTransferTransactionV1.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new Address();
		this._amount = new Amount();
		this._message = null;
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._recipientAddressSize = 40; // reserved field
	}

	sort() {
		if (0 !== this.messageEnvelopeSizeComputed)
			this._message.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get amount() {
		return this._amount;
	}

	set amount(value) {
		this._amount = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get messageEnvelopeSizeComputed() {
		return this.message ? this.message.size + 0 : 0;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.recipientAddress.size;
		size += this.amount.size;
		size += 4;
		if (0 !== this.messageEnvelopeSizeComputed)
			size += this.message.size;

		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddressSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== recipientAddressSize)
			throw RangeError(`Invalid value of reserved field (${recipientAddressSize})`);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const amount = Amount.deserialize(view.buffer);
		view.shiftRight(amount.size);
		const messageEnvelopeSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let message = null;
		if (0 !== messageEnvelopeSize) {
			message = Message.deserialize(view.buffer);
			view.shiftRight(message.size);
		}

		const instance = new NonVerifiableTransferTransactionV1();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._amount = amount;
		instance._message = message;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._recipientAddressSize, 4, false));
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._amount.serialize());
		buffer.write(converter.intToBytes(this.messageEnvelopeSizeComputed, 4, false));
		if (0 !== this.messageEnvelopeSizeComputed)
			buffer.write(this._message.serialize());

		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `amount: ${this._amount.toString()}, `;
		if (0 !== this.messageEnvelopeSizeComputed)
			result += `message: ${this._message.toString()}, `;

		result += ')';
		return result;
	}
}

export class TransferTransactionV2 {
	static TRANSACTION_VERSION = 2;

	static TRANSACTION_TYPE = TransactionType.TRANSFER;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		signature: 'pod:Signature',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:Address',
		amount: 'pod:Amount',
		message: 'struct:Message',
		mosaics: 'array[SizePrefixedMosaic]'
	};

	constructor() {
		this._type = TransferTransactionV2.TRANSACTION_TYPE;
		this._version = TransferTransactionV2.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._signature = new Signature();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new Address();
		this._amount = new Amount();
		this._message = null;
		this._mosaics = [];
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._signatureSize = 64; // reserved field
		this._recipientAddressSize = 40; // reserved field
	}

	sort() {
		if (0 !== this.messageEnvelopeSizeComputed)
			this._message.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
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

	get amount() {
		return this._amount;
	}

	set amount(value) {
		this._amount = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get mosaics() {
		return this._mosaics;
	}

	set mosaics(value) {
		this._mosaics = value;
	}

	get messageEnvelopeSizeComputed() {
		return this.message ? this.message.size + 0 : 0;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += 4;
		size += this.signature.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.recipientAddress.size;
		size += this.amount.size;
		size += 4;
		if (0 !== this.messageEnvelopeSizeComputed)
			size += this.message.size;

		size += 4;
		size += arrayHelpers.size(this.mosaics);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const signatureSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (64 !== signatureSize)
			throw RangeError(`Invalid value of reserved field (${signatureSize})`);
		const signature = Signature.deserialize(view.buffer);
		view.shiftRight(signature.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddressSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== recipientAddressSize)
			throw RangeError(`Invalid value of reserved field (${recipientAddressSize})`);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const amount = Amount.deserialize(view.buffer);
		view.shiftRight(amount.size);
		const messageEnvelopeSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let message = null;
		if (0 !== messageEnvelopeSize) {
			message = Message.deserialize(view.buffer);
			view.shiftRight(message.size);
		}
		const mosaicsCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const mosaics = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMosaic, mosaicsCount);
		view.shiftRight(arrayHelpers.size(mosaics));

		const instance = new TransferTransactionV2();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._signature = signature;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._amount = amount;
		instance._message = message;
		instance._mosaics = mosaics;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(converter.intToBytes(this._signatureSize, 4, false));
		buffer.write(this._signature.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._recipientAddressSize, 4, false));
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._amount.serialize());
		buffer.write(converter.intToBytes(this.messageEnvelopeSizeComputed, 4, false));
		if (0 !== this.messageEnvelopeSizeComputed)
			buffer.write(this._message.serialize());

		buffer.write(converter.intToBytes(this._mosaics.length, 4, false)); // bound: mosaics_count
		arrayHelpers.writeArray(buffer, this._mosaics);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `signature: ${this._signature.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `amount: ${this._amount.toString()}, `;
		if (0 !== this.messageEnvelopeSizeComputed)
			result += `message: ${this._message.toString()}, `;

		result += `mosaics: [${this._mosaics.map(e => e.toString()).join(',')}], `;
		result += ')';
		return result;
	}
}

export class NonVerifiableTransferTransactionV2 {
	static TRANSACTION_VERSION = 2;

	static TRANSACTION_TYPE = TransactionType.TRANSFER;

	static TYPE_HINTS = {
		type: 'enum:TransactionType',
		network: 'enum:NetworkType',
		timestamp: 'pod:Timestamp',
		signerPublicKey: 'pod:PublicKey',
		fee: 'pod:Amount',
		deadline: 'pod:Timestamp',
		recipientAddress: 'pod:Address',
		amount: 'pod:Amount',
		message: 'struct:Message',
		mosaics: 'array[SizePrefixedMosaic]'
	};

	constructor() {
		this._type = NonVerifiableTransferTransactionV2.TRANSACTION_TYPE;
		this._version = NonVerifiableTransferTransactionV2.TRANSACTION_VERSION;
		this._network = NetworkType.MAINNET;
		this._timestamp = new Timestamp();
		this._signerPublicKey = new PublicKey();
		this._fee = new Amount();
		this._deadline = new Timestamp();
		this._recipientAddress = new Address();
		this._amount = new Amount();
		this._message = null;
		this._mosaics = [];
		this._entityBodyReserved_1 = 0; // reserved field
		this._signerPublicKeySize = 32; // reserved field
		this._recipientAddressSize = 40; // reserved field
	}

	sort() {
		if (0 !== this.messageEnvelopeSizeComputed)
			this._message.sort();
	}

	get type() {
		return this._type;
	}

	set type(value) {
		this._type = value;
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

	get timestamp() {
		return this._timestamp;
	}

	set timestamp(value) {
		this._timestamp = value;
	}

	get signerPublicKey() {
		return this._signerPublicKey;
	}

	set signerPublicKey(value) {
		this._signerPublicKey = value;
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

	get amount() {
		return this._amount;
	}

	set amount(value) {
		this._amount = value;
	}

	get message() {
		return this._message;
	}

	set message(value) {
		this._message = value;
	}

	get mosaics() {
		return this._mosaics;
	}

	set mosaics(value) {
		this._mosaics = value;
	}

	get messageEnvelopeSizeComputed() {
		return this.message ? this.message.size + 0 : 0;
	}

	get size() { // eslint-disable-line class-methods-use-this
		let size = 0;
		size += this.type.size;
		size += 1;
		size += 2;
		size += this.network.size;
		size += this.timestamp.size;
		size += 4;
		size += this.signerPublicKey.size;
		size += this.fee.size;
		size += this.deadline.size;
		size += 4;
		size += this.recipientAddress.size;
		size += this.amount.size;
		size += 4;
		if (0 !== this.messageEnvelopeSizeComputed)
			size += this.message.size;

		size += 4;
		size += arrayHelpers.size(this.mosaics);
		return size;
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const type = TransactionType.deserialize(view.buffer);
		view.shiftRight(type.size);
		const version = converter.bytesToIntUnaligned(view.buffer, 1, false);
		view.shiftRight(1);
		const entityBodyReserved_1 = converter.bytesToIntUnaligned(view.buffer, 2, false);
		view.shiftRight(2);
		if (0 !== entityBodyReserved_1)
			throw RangeError(`Invalid value of reserved field (${entityBodyReserved_1})`);
		const network = NetworkType.deserialize(view.buffer);
		view.shiftRight(network.size);
		const timestamp = Timestamp.deserialize(view.buffer);
		view.shiftRight(timestamp.size);
		const signerPublicKeySize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (32 !== signerPublicKeySize)
			throw RangeError(`Invalid value of reserved field (${signerPublicKeySize})`);
		const signerPublicKey = PublicKey.deserialize(view.buffer);
		view.shiftRight(signerPublicKey.size);
		const fee = Amount.deserialize(view.buffer);
		view.shiftRight(fee.size);
		const deadline = Timestamp.deserialize(view.buffer);
		view.shiftRight(deadline.size);
		const recipientAddressSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		if (40 !== recipientAddressSize)
			throw RangeError(`Invalid value of reserved field (${recipientAddressSize})`);
		const recipientAddress = Address.deserialize(view.buffer);
		view.shiftRight(recipientAddress.size);
		const amount = Amount.deserialize(view.buffer);
		view.shiftRight(amount.size);
		const messageEnvelopeSize = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		let message = null;
		if (0 !== messageEnvelopeSize) {
			message = Message.deserialize(view.buffer);
			view.shiftRight(message.size);
		}
		const mosaicsCount = converter.bytesToIntUnaligned(view.buffer, 4, false);
		view.shiftRight(4);
		const mosaics = arrayHelpers.readArrayCount(view.buffer, SizePrefixedMosaic, mosaicsCount);
		view.shiftRight(arrayHelpers.size(mosaics));

		const instance = new NonVerifiableTransferTransactionV2();
		instance._type = type;
		instance._version = version;
		instance._network = network;
		instance._timestamp = timestamp;
		instance._signerPublicKey = signerPublicKey;
		instance._fee = fee;
		instance._deadline = deadline;
		instance._recipientAddress = recipientAddress;
		instance._amount = amount;
		instance._message = message;
		instance._mosaics = mosaics;
		return instance;
	}

	serialize() {
		const buffer = new Writer(this.size);
		buffer.write(this._type.serialize());
		buffer.write(converter.intToBytes(this._version, 1, false));
		buffer.write(converter.intToBytes(this._entityBodyReserved_1, 2, false));
		buffer.write(this._network.serialize());
		buffer.write(this._timestamp.serialize());
		buffer.write(converter.intToBytes(this._signerPublicKeySize, 4, false));
		buffer.write(this._signerPublicKey.serialize());
		buffer.write(this._fee.serialize());
		buffer.write(this._deadline.serialize());
		buffer.write(converter.intToBytes(this._recipientAddressSize, 4, false));
		buffer.write(this._recipientAddress.serialize());
		buffer.write(this._amount.serialize());
		buffer.write(converter.intToBytes(this.messageEnvelopeSizeComputed, 4, false));
		if (0 !== this.messageEnvelopeSizeComputed)
			buffer.write(this._message.serialize());

		buffer.write(converter.intToBytes(this._mosaics.length, 4, false)); // bound: mosaics_count
		arrayHelpers.writeArray(buffer, this._mosaics);
		return buffer.storage;
	}

	toString() {
		let result = '(';
		result += `type: ${this._type.toString()}, `;
		result += `version: ${'0x'.concat(this._version.toString(16))}, `;
		result += `network: ${this._network.toString()}, `;
		result += `timestamp: ${this._timestamp.toString()}, `;
		result += `signerPublicKey: ${this._signerPublicKey.toString()}, `;
		result += `fee: ${this._fee.toString()}, `;
		result += `deadline: ${this._deadline.toString()}, `;
		result += `recipientAddress: ${this._recipientAddress.toString()}, `;
		result += `amount: ${this._amount.toString()}, `;
		if (0 !== this.messageEnvelopeSizeComputed)
			result += `message: ${this._message.toString()}, `;

		result += `mosaics: [${this._mosaics.map(e => e.toString()).join(',')}], `;
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
		mapping.set(TransactionFactory.toKey([MosaicDefinitionTransactionV1.TRANSACTION_TYPE.value, MosaicDefinitionTransactionV1.TRANSACTION_VERSION]), MosaicDefinitionTransactionV1);
		mapping.set(TransactionFactory.toKey([MosaicSupplyChangeTransactionV1.TRANSACTION_TYPE.value, MosaicSupplyChangeTransactionV1.TRANSACTION_VERSION]), MosaicSupplyChangeTransactionV1);
		mapping.set(TransactionFactory.toKey([MultisigAccountModificationTransactionV1.TRANSACTION_TYPE.value, MultisigAccountModificationTransactionV1.TRANSACTION_VERSION]), MultisigAccountModificationTransactionV1);
		mapping.set(TransactionFactory.toKey([MultisigAccountModificationTransactionV2.TRANSACTION_TYPE.value, MultisigAccountModificationTransactionV2.TRANSACTION_VERSION]), MultisigAccountModificationTransactionV2);
		mapping.set(TransactionFactory.toKey([CosignatureV1.TRANSACTION_TYPE.value, CosignatureV1.TRANSACTION_VERSION]), CosignatureV1);
		mapping.set(TransactionFactory.toKey([MultisigTransactionV1.TRANSACTION_TYPE.value, MultisigTransactionV1.TRANSACTION_VERSION]), MultisigTransactionV1);
		mapping.set(TransactionFactory.toKey([NamespaceRegistrationTransactionV1.TRANSACTION_TYPE.value, NamespaceRegistrationTransactionV1.TRANSACTION_VERSION]), NamespaceRegistrationTransactionV1);
		mapping.set(TransactionFactory.toKey([TransferTransactionV1.TRANSACTION_TYPE.value, TransferTransactionV1.TRANSACTION_VERSION]), TransferTransactionV1);
		mapping.set(TransactionFactory.toKey([TransferTransactionV2.TRANSACTION_TYPE.value, TransferTransactionV2.TRANSACTION_VERSION]), TransferTransactionV2);
		const discriminator = TransactionFactory.toKey([parent.type.value, parent.version]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			account_key_link_transaction_v1: AccountKeyLinkTransactionV1,
			mosaic_definition_transaction_v1: MosaicDefinitionTransactionV1,
			mosaic_supply_change_transaction_v1: MosaicSupplyChangeTransactionV1,
			multisig_account_modification_transaction_v1: MultisigAccountModificationTransactionV1,
			multisig_account_modification_transaction_v2: MultisigAccountModificationTransactionV2,
			cosignature_v1: CosignatureV1,
			multisig_transaction_v1: MultisigTransactionV1,
			namespace_registration_transaction_v1: NamespaceRegistrationTransactionV1,
			transfer_transaction_v1: TransferTransactionV1,
			transfer_transaction_v2: TransferTransactionV2
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError(`unknown Transaction type ${entityName}`);

		return new mapping[entityName]();
	}
}

export class NonVerifiableTransactionFactory {
	static toKey(values) {
		if (1 === values.length)
			return values[0];

		// assume each key is at most 32bits
		return values.map(n => BigInt(n)).reduce((accumulator, value) => (accumulator << 32n) + value);
	}

	static deserialize(payload) {
		const view = new BufferView(payload);
		const parent = NonVerifiableTransaction.deserialize(view.buffer);
		const mapping = new Map();
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableAccountKeyLinkTransactionV1.TRANSACTION_TYPE.value, NonVerifiableAccountKeyLinkTransactionV1.TRANSACTION_VERSION]), NonVerifiableAccountKeyLinkTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableMosaicDefinitionTransactionV1.TRANSACTION_TYPE.value, NonVerifiableMosaicDefinitionTransactionV1.TRANSACTION_VERSION]), NonVerifiableMosaicDefinitionTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableMosaicSupplyChangeTransactionV1.TRANSACTION_TYPE.value, NonVerifiableMosaicSupplyChangeTransactionV1.TRANSACTION_VERSION]), NonVerifiableMosaicSupplyChangeTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableMultisigAccountModificationTransactionV1.TRANSACTION_TYPE.value, NonVerifiableMultisigAccountModificationTransactionV1.TRANSACTION_VERSION]), NonVerifiableMultisigAccountModificationTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableMultisigAccountModificationTransactionV2.TRANSACTION_TYPE.value, NonVerifiableMultisigAccountModificationTransactionV2.TRANSACTION_VERSION]), NonVerifiableMultisigAccountModificationTransactionV2);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableMultisigTransactionV1.TRANSACTION_TYPE.value, NonVerifiableMultisigTransactionV1.TRANSACTION_VERSION]), NonVerifiableMultisigTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableNamespaceRegistrationTransactionV1.TRANSACTION_TYPE.value, NonVerifiableNamespaceRegistrationTransactionV1.TRANSACTION_VERSION]), NonVerifiableNamespaceRegistrationTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableTransferTransactionV1.TRANSACTION_TYPE.value, NonVerifiableTransferTransactionV1.TRANSACTION_VERSION]), NonVerifiableTransferTransactionV1);
		mapping.set(NonVerifiableTransactionFactory.toKey([NonVerifiableTransferTransactionV2.TRANSACTION_TYPE.value, NonVerifiableTransferTransactionV2.TRANSACTION_VERSION]), NonVerifiableTransferTransactionV2);
		const discriminator = NonVerifiableTransactionFactory.toKey([parent.type.value, parent.version]);
		const factory_class = mapping.get(discriminator);
		return factory_class.deserialize(view.buffer);
	}

	static createByName(entityName) {
		const mapping = {
			non_verifiable_account_key_link_transaction_v1: NonVerifiableAccountKeyLinkTransactionV1,
			non_verifiable_mosaic_definition_transaction_v1: NonVerifiableMosaicDefinitionTransactionV1,
			non_verifiable_mosaic_supply_change_transaction_v1: NonVerifiableMosaicSupplyChangeTransactionV1,
			non_verifiable_multisig_account_modification_transaction_v1: NonVerifiableMultisigAccountModificationTransactionV1,
			non_verifiable_multisig_account_modification_transaction_v2: NonVerifiableMultisigAccountModificationTransactionV2,
			non_verifiable_multisig_transaction_v1: NonVerifiableMultisigTransactionV1,
			non_verifiable_namespace_registration_transaction_v1: NonVerifiableNamespaceRegistrationTransactionV1,
			non_verifiable_transfer_transaction_v1: NonVerifiableTransferTransactionV1,
			non_verifiable_transfer_transaction_v2: NonVerifiableTransferTransactionV2
		};

		if (!Object.prototype.hasOwnProperty.call(mapping, entityName))
			throw RangeError(`unknown NonVerifiableTransaction type ${entityName}`);

		return new mapping[entityName]();
	}
}
