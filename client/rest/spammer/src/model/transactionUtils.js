import catapult from 'catapult-sdk';
import networkTime from '../utils/networkTime';
import utils from '../utils/spammerUtils';

const serialize = catapult.modelBinary.serialize;
const verify = catapult.crypto.verify;
const Header_Size = 4 + 64 + 32;
const Signature_Size = 64;

function signTransaction(codec, keyPair, transaction) {
	// buffer is a nodejs buffer that does not copy any data when slicing
	const buffer = serialize.toBuffer(codec, transaction).slice(Header_Size);
	transaction.signature = catapult.crypto.sign(keyPair, buffer);
}

function verifyTransactionSignature(codec, transaction) {
	// buffer is a nodejs buffer that does not copy any data when slicing
	const buffer = serialize.toBuffer(codec, transaction).slice(Header_Size);
	return verify(transaction.signer, buffer, transaction.signature);
}

function createRandomTransfer(options, recipientSelector) {
	return {
		signature: new Uint8Array(Signature_Size),
		signer: options.signerPublicKey,
		version: (options.networkId << 8) + 3,
		type: catapult.model.EntityType.transfer,
		fee: utils.toUint64(0),
		deadline: utils.toUint64(networkTime.getNetworkTime() + (60 * 60 * 1000)),
		recipient: recipientSelector(),
		message: {
			type: 0,
			payload: utils.uint32ToBytes(options.transferId)
		},
		mosaics: [
			{ id: [0xD95FCF29, 0xD525AD41],	amount: utils.toUint64(utils.random(1000000)) }
		]
	};
}

export default { signTransaction, verifyTransactionSignature, createRandomTransfer };
