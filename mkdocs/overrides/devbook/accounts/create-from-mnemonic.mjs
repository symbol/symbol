import { Bip32 } from 'symbol-sdk';
import { SymbolFacade } from 'symbol-sdk/symbol';
import process from 'process';

// Initialize the facade for the testnet network
const facade = new SymbolFacade('testnet');

// Use an existing mnemonic if provided, otherwise generate a random one
const bip32 = new Bip32(facade.constructor.BIP32_CURVE_NAME);
let mnemonic = process.env.MNEMONIC;
if (mnemonic) {
	console.log('Loading mnemonic from environment variable...');
} else {
	console.log('Generating random mnemonic...');
	mnemonic = bip32.random();
}
console.log('Mnemonic:', mnemonic);

// Load password from environment variable or use default
const password = process.env.PASSWORD || 'correcthorsebatterystaple';
console.log('Password:', password);

// Derive a root Bip32 node from the mnemonic and a password
const rootNode = bip32.fromMnemonic(mnemonic, password);

// Derive a child Bip32 node for the account at index 0
const accountIndex = 0;
const childNode = rootNode.derivePath(facade.bip32Path(accountIndex));

// Convert the Bip32 node to a signing key pair
const keyPair = facade.constructor.bip32NodeToKeyPair(childNode);

// Derive the address from the public key
const address = facade.network.publicKeyToAddress(keyPair.publicKey);

// Output the account details
console.log('Address:', address.toString());
console.log('Public key:', keyPair.publicKey.toString());
console.log('Private key:', keyPair.privateKey.toString());
