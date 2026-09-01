//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;

final class CreateFromPrivateKey {
	private CreateFromPrivateKey() {
	}

	public static void main(final String[] args) {
		// Initialize the facade for the testnet network [>step-1]
		final SymbolFacade facade = new SymbolFacade("testnet");
		// [<step-1]
		// Use an existing private key if provided, [>step-2]
		// Otherwise generate a random one.
		final String privateKeyString = System.getenv("PRIVATE_KEY");
		final CryptoTypes.PrivateKey privateKey;
		if (null != privateKeyString) {
			System.out.println(
				"Loading account from environment variable...");
			privateKey = new CryptoTypes.PrivateKey(privateKeyString);
		} else {
			System.out.println("Generating random account...");
			privateKey = CryptoTypes.PrivateKey.random();
		} // [<step-2]
		// Create a key pair from the private key [>step-3]
		final KeyPair keyPair = new KeyPair(privateKey);

		// Derive the public key from the private key
		final CryptoTypes.PublicKey publicKey = keyPair.getPublicKey();

		// Derive the address from the public key
		final Address address = facade.network.publicKeyToAddress(
			publicKey);

		// Output the account details
		System.out.println("Address: " + address);
		System.out.println("Public key: " + publicKey);
		System.out.println("Private key: " + privateKey); // [<step-3]
	}
}
