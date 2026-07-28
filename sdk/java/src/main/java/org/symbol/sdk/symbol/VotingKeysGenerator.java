package org.symbol.sdk.symbol;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.function.Supplier;

import org.symbol.sdk.CryptoTypes;

/**
 * Generates Symbol voting keys. Binary layout: an 80-byte header followed by a 96-byte entry per epoch (private key + parent signature over
 * the child public key concatenated with the epoch identifier).
 */
public final class VotingKeysGenerator {
	private static final int HEADER_SIZE = 80;
	private static final int EPOCH_ENTRY_SIZE = 96;

	private final KeyPair rootKeyPair;
	private final Supplier<CryptoTypes.PrivateKey> privateKeyGenerator;

	/**
	 * Creates a generator around a voting root key pair using {@link CryptoTypes.PrivateKey#random} for child key generation.
	 *
	 * @param rootKeyPair Voting root key pair.
	 */
	public VotingKeysGenerator(final KeyPair rootKeyPair) {
		this(rootKeyPair, CryptoTypes.PrivateKey::random);
	}

	/**
	 * Creates a generator around a voting root key pair with a custom private-key generator.
	 *
	 * @param rootKeyPair Voting root key pair.
	 * @param privateKeyGenerator Private key generator.
	 */
	public VotingKeysGenerator(final KeyPair rootKeyPair, final Supplier<CryptoTypes.PrivateKey> privateKeyGenerator) {
		this.rootKeyPair = rootKeyPair;
		this.privateKeyGenerator = privateKeyGenerator;
	}

	/**
	 * Generates voting keys for the inclusive epoch range {@code [startEpoch, endEpoch]}.
	 *
	 * @param startEpoch Start epoch.
	 * @param endEpoch End epoch.
	 * @return Serialized voting keys.
	 */
	public byte[] generate(final long startEpoch, final long endEpoch) {
		final int numEpochs = Math.toIntExact(endEpoch - startEpoch + 1L);
		// toIntExact bounds numEpochs but not the buffer size; HEADER_SIZE + 96 * numEpochs overflows int for a large range,
		// so compute it with checked arithmetic and reject rather than allocating an undersized buffer
		final int totalSize;
		try {
			totalSize = Math.addExact(HEADER_SIZE, Math.multiplyExact(EPOCH_ENTRY_SIZE, numEpochs));
		} catch (final ArithmeticException ex) {
			throw new IllegalArgumentException(String.format("epoch range too large: %d epochs overflow the voting key buffer", numEpochs),
					ex);
		}

		final ByteBuffer buffer = ByteBuffer.allocate(totalSize).order(ByteOrder.LITTLE_ENDIAN);

		buffer.putLong(0, startEpoch); // start key identifier
		buffer.putLong(8, endEpoch); // end key identifier
		buffer.putLong(16, 0xFFFFFFFFFFFFFFFFL); // reserved - last (used) key identifier
		buffer.putLong(24, 0xFFFFFFFFFFFFFFFFL); // reserved - last wiped key identifier

		System.arraycopy(rootKeyPair.getPublicKey().bytes(), 0, buffer.array(), 32, CryptoTypes.PublicKey.SIZE);
		buffer.putLong(64, startEpoch); // level 1/1 start key identifier
		buffer.putLong(72, endEpoch); // level 1/1 end key identifier

		for (int i = 0; i < numEpochs; ++i) {
			final long identifier = endEpoch - i;
			final CryptoTypes.PrivateKey childPrivateKey = privateKeyGenerator.get();
			final KeyPair childKeyPair = new KeyPair(childPrivateKey);

			final ByteBuffer parentSignedPayload = ByteBuffer.allocate(40).order(ByteOrder.LITTLE_ENDIAN);
			parentSignedPayload.put(childKeyPair.getPublicKey().bytes());
			parentSignedPayload.putLong(32, identifier);

			final CryptoTypes.Signature signature = rootKeyPair.sign(parentSignedPayload.array());

			final int startOffset = HEADER_SIZE + EPOCH_ENTRY_SIZE * i;
			System.arraycopy(childKeyPair.getPrivateKey().bytes(), 0, buffer.array(), startOffset, CryptoTypes.PrivateKey.SIZE);
			System.arraycopy(signature.bytes(), 0, buffer.array(), startOffset + CryptoTypes.PrivateKey.SIZE, CryptoTypes.Signature.SIZE);
		}

		return buffer.array();
	}
}
