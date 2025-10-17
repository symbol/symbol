import { PrivateKey } from 'symbol-sdk';
import { SymbolFacade } from 'symbol-sdk/symbol';
import process from 'process';

// Initialize the facade for the testnet network
const facade = new SymbolFacade('testnet');

// ===== RANDOM ACCOUNT =====
console.log('Creating a random account...');

// Generate a random private key
const randomPrivateKey = PrivateKey.random();

// Create a key pair from the private key
const randomKeyPair = new SymbolFacade.KeyPair(randomPrivateKey);

// Derive the address from the public key
const randomAddress = facade.network.publicKeyToAddress(
	randomKeyPair.publicKey);

// Output the account details
console.log('    Address:', randomAddress.toString());
console.log('    Public key:', randomKeyPair.publicKey.toString());
console.log('    Private key:', randomKeyPair.privateKey.toString());

// ===== ACCOUNT FROM PRIVATE KEY =====
console.log('\nCreating an account from a private key...');

// Load an existing private key from environment variable or use default
const existingPrivateKeyString = process.env.PRIVATE_KEY ||
	'0000000000000000000000000000000000000000000000000000000000000000';
const existingPrivateKey = new PrivateKey(existingPrivateKeyString);

// Create a key pair from the private key
const existingkeyPair = new SymbolFacade.KeyPair(existingPrivateKey);

// Derive the address from the public key
const existingAddress = facade.network.publicKeyToAddress(
	existingkeyPair.publicKey);

// Output the account details
console.log('    Address:', existingAddress.toString());
console.log('    Public key:', existingkeyPair.publicKey.toString());
console.log('    Private key:', existingkeyPair.privateKey.toString());