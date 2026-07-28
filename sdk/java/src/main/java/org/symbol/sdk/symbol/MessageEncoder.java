package org.symbol.sdk.symbol;

import java.util.Arrays;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.MessageEncoderResult;
import org.symbol.sdk.impl.CipherHelpers;
import org.symbol.sdk.utils.ArrayHelpers;
import org.symbol.sdk.utils.Converter;

/**
 * Encrypts and encodes messages between two Symbol parties. Wire format is {@code [0x01 | tag(16) | iv(12) | cipherText]}; persistent
 * harvesting delegations use {@code [FE2A8061577301E2 marker(8) | ephemeralPubKey(32) | tag(16) | iv(12) | cipherText]}.
 */
public final class MessageEncoder {

	private static final byte[] DELEGATION_MARKER = Converter.hexToUint8("FE2A8061577301E2");

	private final KeyPair keyPair;

	/**
	 * Creates a message encoder around a key pair.
	 *
	 * @param keyPair Key pair.
	 */
	public MessageEncoder(final KeyPair keyPair) {
		this.keyPair = keyPair;
	}

	/**
	 * Returns the public key used for message encoding.
	 *
	 * @return Public key used for message encoding.
	 */
	public CryptoTypes.PublicKey getPublicKey() {
		return keyPair.getPublicKey();
	}

	/**
	 * Tries to decode an encoded message.
	 *
	 * @param recipientPublicKey Recipient's public key.
	 * @param encodedMessage Encoded message bytes.
	 * @return Decoded result. {@code isDecoded()} is {@code true} on success.
	 */
	public MessageEncoderResult tryDecode(final CryptoTypes.PublicKey recipientPublicKey, final byte[] encodedMessage) {
		if (0 < encodedMessage.length && 1 == encodedMessage[0]) {
			final byte[] decoded = tryDecodeGcm(recipientPublicKey, Arrays.copyOfRange(encodedMessage, 1, encodedMessage.length));
			if (null != decoded)
				return new MessageEncoderResult(true, decoded);
		}

		if (encodedMessage.length >= DELEGATION_MARKER.length + CryptoTypes.PublicKey.SIZE
				&& Arrays.equals(Arrays.copyOfRange(encodedMessage, 0, DELEGATION_MARKER.length), DELEGATION_MARKER)) {
			// the enclosing length guard guarantees at least DELEGATION_MARKER.length + PublicKey.SIZE bytes, so this slice is
			// always in bounds (copyOfRange cannot throw here) and the ephemeral key is exactly PublicKey.SIZE bytes
			final int ephemeralPublicKeyEnd = DELEGATION_MARKER.length + CryptoTypes.PublicKey.SIZE;
			final byte[] ephemeralPublicKeyBytes = Arrays.copyOfRange(encodedMessage, DELEGATION_MARKER.length, ephemeralPublicKeyEnd);
			final CryptoTypes.PublicKey ephemeralPublicKey = new CryptoTypes.PublicKey(ephemeralPublicKeyBytes);

			final byte[] payload = Arrays.copyOfRange(encodedMessage, ephemeralPublicKeyEnd, encodedMessage.length);
			byte[] decoded;
			try {
				decoded = tryDecodeGcm(ephemeralPublicKey, payload);
			} catch (IllegalArgumentException ex) {
				// invalid curve point — fall through to "not decoded"
				decoded = null;
			}

			if (null != decoded)
				return new MessageEncoderResult(true, decoded);
		}

		return new MessageEncoderResult(false, encodedMessage);
	}

	/**
	 * Encodes a message to a recipient using the recommended format.
	 *
	 * @param recipientPublicKey Recipient public key.
	 * @param message Message to encode.
	 * @return Encrypted and encoded message.
	 */
	public byte[] encode(final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
		final CipherHelpers.EncodedGcm encoded = CipherHelpers.encodeAesGcm(SharedKey.deriveSharedKey(this.keyPair, recipientPublicKey),
				message);
		return ArrayHelpers.concat(new byte[]{
				1
		}, encoded.tag(), encoded.initializationVector(), encoded.cipherText());
	}

	/**
	 * Encodes a persistent harvesting delegation to a node.
	 *
	 * @param nodePublicKey Node public key.
	 * @param remoteKeyPair Remote key pair.
	 * @param vrfKeyPair VRF key pair.
	 * @return Encrypted and encoded harvesting delegation request.
	 */
	public byte[] encodePersistentHarvestingDelegation(final CryptoTypes.PublicKey nodePublicKey, final KeyPair remoteKeyPair,
			final KeyPair vrfKeyPair) {
		final KeyPair ephemeralKeyPair = new KeyPair(CryptoTypes.PrivateKey.random());
		final byte[] message = ArrayHelpers.concat(remoteKeyPair.getPrivateKey().bytes(), vrfKeyPair.getPrivateKey().bytes());
		final CipherHelpers.EncodedGcm encoded = CipherHelpers.encodeAesGcm(SharedKey.deriveSharedKey(ephemeralKeyPair, nodePublicKey),
				message);

		return ArrayHelpers.concat(DELEGATION_MARKER, ephemeralKeyPair.getPublicKey().bytes(), encoded.tag(),
				encoded.initializationVector(), encoded.cipherText());
	}

	/**
	 * Tries to decode an encoded message using the deprecated wallet format; provided only for compatibility with the original Symbol
	 * wallets — new code should use {@link #tryDecode(CryptoTypes.PublicKey, byte[])}.
	 *
	 * @param recipientPublicKey Recipient's public key.
	 * @param encodedMessage Encoded message bytes.
	 * @return Decoded result.
	 */
	public MessageEncoderResult tryDecodeDeprecated(final CryptoTypes.PublicKey recipientPublicKey, final byte[] encodedMessage) {
		if (0 < encodedMessage.length && 1 == encodedMessage[0]) {
			// wallet additionally hex encodes the payload
			final String encodedHexString = new String(Arrays.copyOfRange(encodedMessage, 1, encodedMessage.length),
					java.nio.charset.StandardCharsets.UTF_8);
			if (Converter.isHexString(encodedHexString)) {
				final byte[] reencoded = ArrayHelpers.concat(new byte[]{
						1
				}, Converter.hexToUint8(encodedHexString));
				return tryDecode(recipientPublicKey, reencoded);
			}
		}

		return tryDecode(recipientPublicKey, encodedMessage);
	}

	/**
	 * Encodes a message to a recipient using the deprecated wallet format; provided only for compatibility with the original Symbol wallets
	 * — new code should use {@link #encode(CryptoTypes.PublicKey, byte[])}.
	 *
	 * @param recipientPublicKey Recipient public key.
	 * @param message Message to encode.
	 * @return Encrypted and encoded message.
	 */
	public byte[] encodeDeprecated(final CryptoTypes.PublicKey recipientPublicKey, final byte[] message) {
		final byte[] encoded = encode(recipientPublicKey, message);
		// wallet additionally hex encodes the (payload-after-leading-0x01)
		final String hex = Converter.uint8ToHex(Arrays.copyOfRange(encoded, 1, encoded.length));
		final byte[] hexBytes = hex.getBytes(java.nio.charset.StandardCharsets.UTF_8);
		return ArrayHelpers.concat(new byte[]{
				1
		}, hexBytes);
	}

	private byte[] tryDecodeGcm(final CryptoTypes.PublicKey otherPublicKey, final byte[] tagIvCipher) {
		// derive the shared key before the helper's length guard, so an off-curve key throws
		// "invalid point" even for a short payload rather than being silently reported as not-decoded
		final CryptoTypes.SharedKey256 sharedKey = SharedKey.deriveSharedKey(this.keyPair, otherPublicKey);
		return CipherHelpers.tryDecodeAesGcm(sharedKey, tagIvCipher);
	}
}
