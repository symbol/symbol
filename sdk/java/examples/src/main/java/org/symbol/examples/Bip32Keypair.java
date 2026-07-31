package org.symbol.examples;

import java.util.Arrays;

import org.symbol.sdk.Bip32;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;

/**
 * Derives Symbol key pairs from a mnemonic via {@link Bip32}: the account-0 BIP32 path's trailing change/index components are varied to
 * derive keys for branches 0-1 and indexes 0-2.
 */
public final class Bip32Keypair {
	private Bip32Keypair() {
	}

	private static void deriveKey(final Bip32.Bip32Node rootNode, final SymbolFacade facade, final int change, final int index) {
		final int[] path = facade.bip32Path(0);
		path[path.length - 2] = change;
		path[path.length - 1] = index;

		final Bip32.Bip32Node childNode = rootNode.derivePath(path);
		final KeyPair childKeyPair = SymbolFacade.bip32NodeToKeyPair(childNode);

		System.out.println(" PATH: " + Arrays.toString(path));
		System.out.println(" * private key: " + childKeyPair.getPrivateKey());
		System.out.println(" *  public key: " + childKeyPair.getPublicKey());

		final Address address = facade.network.publicKeyToAddress(childKeyPair.getPublicKey());
		System.out.println(" *     address: " + address);
		System.out.println();
	}

	public static void main(final String[] args) {
		final SymbolFacade facade = new SymbolFacade("testnet");

		final Bip32 bip = new Bip32(SymbolFacade.BIP32_CURVE_NAME);
		final Bip32.Bip32Node rootNode = bip.fromMnemonic(
			"cat swing flag economy stadium alone churn speed unique patch report train",
			"correcthorsebatterystaple");

		for (int change = 0; change <= 1; ++change) {
			for (int index = 0; index <= 2; ++index)
				deriveKey(rootNode, facade, change, index);
		}
	}
}
