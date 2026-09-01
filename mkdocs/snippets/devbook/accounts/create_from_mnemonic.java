//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import org.symbol.sdk.Bip32;
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;

final class CreateFromMnemonic {
	private CreateFromMnemonic() {
	}

	public static void main(final String[] args) {
		// Initialize the facade for the testnet network [>step-1]
		final SymbolFacade facade = new SymbolFacade("testnet");
		// [<step-1]
		// Use an existing mnemonic if provided, [>step-2]
		// otherwise generate a random one.
		final Bip32 bip32 = new Bip32(SymbolFacade.BIP32_CURVE_NAME);
		String mnemonic = System.getenv("MNEMONIC");
		if (null != mnemonic) {
			System.out.println(
				"Loading mnemonic phrase from environment variable...");
		} else {
			System.out.println("Generating random mnemonic phrase...");
			mnemonic = bip32.random();
		}
		System.out.println("Mnemonic phrase: " + mnemonic);
		// [<step-2]
		// Load password from environment variable or use default [>step-3]
		final String password = System.getenv().getOrDefault(
			"PASSWORD", "correcthorsebatterystaple");
		System.out.println("Password: " + password);

		// Derive a root Bip32 node from the mnemonic and a password
		final Bip32.Bip32Node rootNode = bip32.fromMnemonic(
			mnemonic, password);
		// [<step-3]
		// Derive a child Bip32 node for the account at index 0 [>step-4]
		final int accountIndex = 0;
		final Bip32.Bip32Node childNode = rootNode.derivePath(
			facade.bip32Path(accountIndex));
		// [<step-4]
		// Convert the Bip32 node to a signing key pair [>step-5]
		final KeyPair keyPair = SymbolFacade.bip32NodeToKeyPair(childNode);

		// Derive the address from the public key
		final CryptoTypes.PublicKey publicKey = keyPair.getPublicKey();
		final Address address = facade.network.publicKeyToAddress(
			publicKey);

		// Output the account details
		System.out.println("Address: " + address);
		System.out.println("Public key: " + publicKey);
		System.out.println(
			"Private key: " + keyPair.getPrivateKey()); // [<step-5]
	}
}
