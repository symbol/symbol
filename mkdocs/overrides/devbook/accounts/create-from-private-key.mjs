import { PrivateKey } from 'symbol-sdk';
import { SymbolFacade } from 'symbol-sdk/symbol';
import process from 'process';

// Initialize the facade for the testnet network
const facade = new SymbolFacade('testnet');

// Use an existing private key if provided,
// Otherwise generate a random one.
const privateKeyString = process.env.PRIVATE_KEY;
let privateKey;
if (privateKeyString) {
	console.log('Loading account from environment variable...');
	privateKey = new PrivateKey(privateKeyString);
} else {
	console.log('Generating random account...');
	privateKey = PrivateKey.random();
}

// Create a key pair from the private key
const keyPair = new SymbolFacade.KeyPair(privateKey);

// Derive the public key from the private key
const publicKey = keyPair.publicKey;

// Derive the address from the public key
const address = facade.network.publicKeyToAddress(publicKey);

// Output the account details
console.log('Address:', address.toString());
console.log('Public key:', publicKey.toString());
console.log('Private key:', privateKey.toString());
