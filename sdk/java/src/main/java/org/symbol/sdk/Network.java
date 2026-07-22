package org.symbol.sdk;

import java.time.Instant;
import java.util.Arrays;
import java.util.List;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.stream.Collectors;

import org.bouncycastle.crypto.Digest;
import org.bouncycastle.crypto.digests.RIPEMD160Digest;

import org.symbol.sdk.utils.Base32;

/**
 * Represents a network.
 *
 * @param <TAddress> Address type.
 * @param <TNetworkTimestamp> Network timestamp type.
 */
public class Network<TAddress extends ByteArray, TNetworkTimestamp extends NetworkTimestamp.Base> {
	/**
	 * Network name.
	 */
	public final String name;

	/**
	 * Network identifier byte.
	 */
	public final byte identifier;

	/**
	 * Network timestamp datetime converter associated with this network.
	 */
	private final NetworkTimestamp.NetworkTimestampDatetimeConverter datetimeConverter;

	private final Supplier<Digest> addressHasher;
	private final BiFunction<byte[], byte[], TAddress> createAddress;
	private final int addressEncodedSize;
	private final Function<String, TAddress> addressFromString;
	private final Function<Long, TNetworkTimestamp> networkTimestampFactory;

	/**
	 * Creates a new network with the specified name and identifier byte.
	 *
	 * @param name Network name.
	 * @param identifier Network identifier byte.
	 * @param datetimeConverter Network timestamp datetime converter associated with this network.
	 * @param addressHasher Factory of the primary hasher to use in the public-key-to-address conversion.
	 * @param createAddress Creates an encoded address from an address-without-checksum and checksum bytes.
	 * @param addressEncodedSize Encoded address size of the address class.
	 * @param addressFromString Constructs an address instance from an encoded base32 string.
	 * @param networkTimestampFactory Constructs a network timestamp from a raw value.
	 */
	public Network(final String name, final byte identifier, final NetworkTimestamp.NetworkTimestampDatetimeConverter datetimeConverter,
			final Supplier<Digest> addressHasher, final BiFunction<byte[], byte[], TAddress> createAddress, final int addressEncodedSize,
			final Function<String, TAddress> addressFromString, final Function<Long, TNetworkTimestamp> networkTimestampFactory) {
		this.name = name;
		this.identifier = identifier;
		this.datetimeConverter = datetimeConverter;
		this.addressHasher = addressHasher;
		this.createAddress = createAddress;
		this.addressEncodedSize = addressEncodedSize;
		this.addressFromString = addressFromString;
		this.networkTimestampFactory = networkTimestampFactory;
	}

	private static byte[] digestAndReset(final Digest digest) {
		final byte[] out = new byte[digest.getDigestSize()];
		digest.doFinal(out, 0);
		return out;
	}

	/**
	 * Converts a public key to an address.
	 *
	 * @param publicKey Public key to convert.
	 * @return Address corresponding to the public key input.
	 */
	public TAddress publicKeyToAddress(final CryptoTypes.PublicKey publicKey) {
		final Digest partOne = addressHasher.get();
		partOne.update(publicKey.bytes(), 0, publicKey.bytes().length);
		final byte[] partOneHash = digestAndReset(partOne);

		final RIPEMD160Digest partTwo = new RIPEMD160Digest();
		partTwo.update(partOneHash, 0, partOneHash.length);
		final byte[] partTwoHash = digestAndReset(partTwo);

		final byte[] version = new byte[1 + partTwoHash.length];
		version[0] = identifier;
		System.arraycopy(partTwoHash, 0, version, 1, partTwoHash.length);

		final Digest partThree = addressHasher.get();
		partThree.update(version, 0, version.length);
		final byte[] checksum = Arrays.copyOf(digestAndReset(partThree), 4);

		return createAddress.apply(version, checksum);
	}

	/**
	 * Checks if an address string is valid and belongs to this network.
	 *
	 * @param addressString Address to check.
	 * @return {@code true} if address is valid and belongs to this network.
	 */
	public boolean isValidAddressString(final String addressString) {
		if (addressEncodedSize != addressString.length())
			return false;

		for (int i = 0; i < addressString.length(); ++i) {
			if (!Base32.isValidChar(addressString.charAt(i)))
				return false;
		}

		return isValidAddress(addressFromString.apply(addressString));
	}

	/**
	 * Checks if an address is valid and belongs to this network.
	 *
	 * @param address Address to check.
	 * @return {@code true} if address is valid and belongs to this network.
	 */
	public boolean isValidAddress(final TAddress address) {
		final byte[] bytes = address.bytes();
		if (bytes[0] != identifier) {
			return false;
		}

		final int hashLength = 21; // 1 + 20
		final Digest hasher = addressHasher.get();
		hasher.update(bytes, 0, hashLength);
		final byte[] expectedHash = digestAndReset(hasher);

		final int checksumLength = bytes.length - hashLength;
		return Arrays.mismatch(bytes, hashLength, bytes.length, expectedHash, 0, checksumLength) == -1;
	}

	/**
	 * Converts a network timestamp to a datetime.
	 *
	 * @param referenceNetworkTimestamp Reference network timestamp to convert.
	 * @return Datetime representation of the reference network timestamp.
	 */
	public Instant toDatetime(final TNetworkTimestamp referenceNetworkTimestamp) {
		return datetimeConverter.toDatetime(referenceNetworkTimestamp.timestamp);
	}

	/**
	 * Converts a datetime to a network timestamp.
	 *
	 * @param referenceDatetime Reference datetime to convert.
	 * @return Network timestamp representation of the reference datetime.
	 */
	public TNetworkTimestamp fromDatetime(final Instant referenceDatetime) {
		return networkTimestampFactory.apply(datetimeConverter.toDifference(referenceDatetime));
	}

	@Override
	public String toString() {
		return name;
	}

	/**
	 * Provides utility functions for finding a network.
	 */
	public static final class NetworkLocator {
		private NetworkLocator() {
		}

		/**
		 * Finds a network with one of the specified names within a list of networks.
		 *
		 * @param <TNetwork> Network type.
		 * @param networks List of networks to search.
		 * @param names Names for which to search.
		 * @return First network with a name in the supplied list.
		 */
		public static <TNetwork extends Network<?, ?>> TNetwork findByName(final List<TNetwork> networks, final List<String> names) {
			return networks.stream().filter(network -> names.contains(network.name)).findFirst().orElseThrow(
					() -> new IllegalArgumentException(String.format("no network found with name '%s'", String.join(", ", names))));
		}

		/**
		 * Find a network by a single name.
		 *
		 * @param <TNetwork> Network type.
		 * @param networks List of networks to search.
		 * @param name Name for which to search.
		 * @return First network with a matching name.
		 */
		public static <TNetwork extends Network<?, ?>> TNetwork findByName(final List<TNetwork> networks, final String name) {
			return findByName(networks, List.of(name));
		}

		/**
		 * Finds a network with one of the specified identifiers within a list of networks.
		 *
		 * @param <TNetwork> Network type.
		 * @param networks List of networks to search.
		 * @param identifiers Identifiers for which to search.
		 * @return First network with an identifier in the supplied list.
		 */
		public static <TNetwork extends Network<?, ?>> TNetwork findByIdentifier(final List<TNetwork> networks,
				final List<Byte> identifiers) {
			return networks.stream().filter(network -> identifiers.contains(network.identifier)).findFirst().orElseThrow(() -> {
				final String ids = identifiers.stream().map(String::valueOf).collect(Collectors.joining(", "));
				return new IllegalArgumentException(String.format("no network found with identifier '%s'", ids));
			});
		}

		/**
		 * Convenience overload finding a network by a single identifier.
		 *
		 * @param <TNetwork> Network type.
		 * @param networks List of networks to search.
		 * @param identifier Identifier for which to search.
		 * @return First network with a matching identifier.
		 */
		public static <TNetwork extends Network<?, ?>> TNetwork findByIdentifier(final List<TNetwork> networks, final byte identifier) {
			return findByIdentifier(networks, List.of(identifier));
		}
	}
}
