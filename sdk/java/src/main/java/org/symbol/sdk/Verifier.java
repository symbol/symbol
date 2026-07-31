package org.symbol.sdk;

/**
 * Cross-blockchain verifier contract implemented by the Symbol and NEM verifiers (which differ only in the Ed25519 hash mode). Obtain one
 * through {@code BlockchainFacade.createVerifier} to stay blockchain-agnostic.
 */
public interface Verifier {
	/**
	 * Verifies a message signature.
	 *
	 * @param message Message to verify.
	 * @param signature Signature to verify.
	 * @return Whether the signature verifies.
	 */
	boolean verify(byte[] message, CryptoTypes.Signature signature);
}
