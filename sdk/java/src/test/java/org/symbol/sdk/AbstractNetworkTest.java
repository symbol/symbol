package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

import java.util.List;
import java.util.function.Function;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.Network.NetworkLocator;

/**
 * Shared {@link Network} / address contract tests run against both the Symbol and NEM networks; subclasses supply the network instances,
 * address vectors, and address factories. Resolution-specific tests (datetime conversion, timestamp arithmetic) and network-specific
 * behavior (e.g. Symbol address aliasing) live in the concrete subclasses.
 *
 * @param <TAddress> Concrete address type.
 * @param <TNetworkTimestamp> Concrete network-timestamp type.
 */
public abstract class AbstractNetworkTest<TAddress extends ByteArray, TNetworkTimestamp extends NetworkTimestamp.Base> {
	/** Address test vector: a public key with its mainnet and testnet encoded addresses. */
	public record AddressVector(String publicKey, String mainnet, String testnet) {
	}

	/** @return Mainnet network under test. */
	protected abstract Network<TAddress, TNetworkTimestamp> mainnet();

	/** @return Testnet network under test. */
	protected abstract Network<TAddress, TNetworkTimestamp> testnet();

	/** @return All networks for this blockchain (for NetworkLocator lookups). */
	protected abstract List<? extends Network<TAddress, TNetworkTimestamp>> networks();

	/** @return Address vectors for this blockchain. */
	protected abstract AddressVector[] addressVectors();

	/**
	 * @param addressString Encoded address string.
	 * @return Address parsed from the string.
	 */
	protected abstract TAddress addressFromString(String addressString);

	/**
	 * @param addressBytes Raw address bytes.
	 * @return Address wrapping the bytes.
	 */
	protected abstract TAddress addressFromBytes(byte[] addressBytes);

	// region publicKeyToAddress

	private void assertCanConvertPublicKeyToAddress(final Network<TAddress, TNetworkTimestamp> network,
			final Function<AddressVector, String> expectedAddress) {
		for (final AddressVector v : addressVectors()) {
			// Arrange:
			final CryptoTypes.PublicKey publicKey = new CryptoTypes.PublicKey(v.publicKey());

			// Act:
			final String address = network.publicKeyToAddress(publicKey).toString();

			// Assert:
			assertThat(address, equalTo(expectedAddress.apply(v)));
		}
	}

	@Test
	void canConvertMainnetPublicKeyToAddress() {
		assertCanConvertPublicKeyToAddress(mainnet(), AddressVector::mainnet);
	}

	@Test
	void canConvertTestnetPublicKeyToAddress() {
		assertCanConvertPublicKeyToAddress(testnet(), AddressVector::testnet);
	}

	@Test
	void addressRoundTripsThroughString() {
		for (final AddressVector v : addressVectors()) {
			// Act:
			final String mainnetRoundTripped = addressFromString(v.mainnet()).toString();
			final String testnetRoundTripped = addressFromString(v.testnet()).toString();

			// Assert:
			assertThat(mainnetRoundTripped, equalTo(v.mainnet()));
			assertThat(testnetRoundTripped, equalTo(v.testnet()));
		}
	}

	// endregion

	// region isValidAddress[String]

	private TAddress deterministicAddress(final Network<TAddress, TNetworkTimestamp> network) {
		return network.publicKeyToAddress(new CryptoTypes.PublicKey(addressVectors()[0].publicKey()));
	}

	private void assertCanValidateValidAddress(final Network<TAddress, TNetworkTimestamp> network) {
		// Arrange:
		final TAddress address = deterministicAddress(network);

		// Act:
		final boolean isValidAddress = network.isValidAddress(address);
		final boolean isValidAddressString = network.isValidAddressString(address.toString());

		// Assert:
		assertThat(isValidAddress, is(true));
		assertThat(isValidAddressString, is(true));
	}

	private void assertCannotValidateInvalidAddress(final Network<TAddress, TNetworkTimestamp> network, final int signedPosition) {
		// Arrange:
		final byte[] tamperedBytes = deterministicAddress(network).bytes().clone();
		final int position = 0 > signedPosition ? tamperedBytes.length + signedPosition : signedPosition;
		tamperedBytes[position] ^= (byte) 0xFF;
		final TAddress tampered = addressFromBytes(tamperedBytes);

		// Act:
		final boolean isValidAddress = network.isValidAddress(tampered);
		final boolean isValidAddressString = network.isValidAddressString(tampered.toString());

		// Assert:
		assertThat(isValidAddress, is(false));
		assertThat(isValidAddressString, is(false));
	}

	private void assertCannotValidateInvalidAddressString(final Network<TAddress, TNetworkTimestamp> network,
			final Function<String, String> mutator) {
		// Arrange:
		final String addressString = mutator.apply(deterministicAddress(network).toString());

		// Act:
		final boolean isValid = network.isValidAddressString(addressString);

		// Assert:
		assertThat(isValid, is(false));
	}

	@Test
	void canValidateValidMainnetAddress() {
		assertCanValidateValidAddress(mainnet());
	}

	@Test
	void canValidateValidTestnetAddress() {
		assertCanValidateValidAddress(testnet());
	}

	@Test
	void cannotValidateInvalidMainnetAddressBegin() {
		assertCannotValidateInvalidAddress(mainnet(), 1);
	}

	@Test
	void cannotValidateInvalidMainnetAddressEnd() {
		assertCannotValidateInvalidAddress(mainnet(), -1);
	}

	@Test
	void cannotValidateInvalidTestnetAddressBegin() {
		assertCannotValidateInvalidAddress(testnet(), 1);
	}

	@Test
	void cannotValidateInvalidTestnetAddressEnd() {
		assertCannotValidateInvalidAddress(testnet(), -1);
	}

	@Test
	void cannotValidateInvalidMainnetAddressStringInvalidSize() {
		assertCannotValidateInvalidAddressString(mainnet(), addressString -> addressString + "A");
		assertCannotValidateInvalidAddressString(mainnet(), addressString -> addressString.substring(0, addressString.length() - 1));
		// (Java-only) empty string
		assertCannotValidateInvalidAddressString(mainnet(), addressString -> "");
	}

	@Test
	void cannotValidateInvalidTestnetAddressStringInvalidSize() {
		assertCannotValidateInvalidAddressString(testnet(), addressString -> addressString + "A");
		assertCannotValidateInvalidAddressString(testnet(), addressString -> addressString.substring(0, addressString.length() - 1));
	}

	@Test
	void cannotValidateInvalidMainnetAddressStringInvalidChar() {
		assertCannotValidateInvalidAddressString(mainnet(),
				addressString -> addressString.substring(0, 10) + "@" + addressString.substring(11));
		// (Java-only) invalid base32 char at the end (length kept valid)
		assertCannotValidateInvalidAddressString(mainnet(), addressString -> addressString.substring(0, addressString.length() - 1) + "!");
	}

	@Test
	void cannotValidateInvalidTestnetAddressStringInvalidChar() {
		assertCannotValidateInvalidAddressString(testnet(),
				addressString -> addressString.substring(0, 10) + "@" + addressString.substring(11));
	}

	// (Java-only) cross-network validation: an address is only valid on the network that derived it
	@Test
	void mainnetRejectsTestnetAddress() {
		assertThat(mainnet().isValidAddressString(addressVectors()[0].testnet()), is(false));
	}

	@Test
	void testnetRejectsMainnetAddress() {
		assertThat(testnet().isValidAddressString(addressVectors()[0].mainnet()), is(false));
	}

	// endregion

	// region NetworkLocatorC

	@Nested
	class NetworkLocatorTest {
		private void assertFindsByName(final String name, final Network<TAddress, TNetworkTimestamp> expected) {
			// Act:
			final Network<TAddress, TNetworkTimestamp> found = NetworkLocator.findByName(networks(), name);

			// Assert:
			assertThat(found, equalTo(expected));
		}

		private void assertFindsByIdentifier(final byte identifier, final Network<TAddress, TNetworkTimestamp> expected) {
			// Act:
			final Network<TAddress, TNetworkTimestamp> found = NetworkLocator.findByIdentifier(networks(), identifier);

			// Assert:
			assertThat(found, equalTo(expected));
		}

		@Test
		void findsMainnetByName() {
			assertFindsByName("mainnet", mainnet());
		}

		@Test
		void findsTestnetByName() {
			assertFindsByName("testnet", testnet());
		}

		@Test
		void findsMainnetByIdentifier() {
			assertFindsByIdentifier((byte) 0x68, mainnet());
		}

		@Test
		void findsTestnetByIdentifier() {
			assertFindsByIdentifier((byte) 0x98, testnet());
		}
	}

	// endregion
}
