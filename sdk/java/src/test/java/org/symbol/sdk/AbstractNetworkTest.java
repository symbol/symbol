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

	@Nested
	class PublicKeyToAddress {
		private void assertDerivesExpectedAddress(final Network<TAddress, TNetworkTimestamp> network,
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
		void derivesMainnetAddress() {
			assertDerivesExpectedAddress(mainnet(), AddressVector::mainnet);
		}

		@Test
		void derivesTestnetAddress() {
			assertDerivesExpectedAddress(testnet(), AddressVector::testnet);
		}
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

	@Nested
	class IsValidAddressString {
		private void assertIsValidAddressString(final Network<TAddress, TNetworkTimestamp> network, final String addressString,
				final boolean expected) {
			// Act:
			final boolean isValid = network.isValidAddressString(addressString);

			// Assert:
			assertThat(isValid, is(expected));
		}

		@Test
		void mainnetAcceptsMainnetAddress() {
			assertIsValidAddressString(mainnet(), addressVectors()[0].mainnet(), true);
		}

		@Test
		void testnetAcceptsTestnetAddress() {
			assertIsValidAddressString(testnet(), addressVectors()[0].testnet(), true);
		}

		@Test
		void mainnetRejectsTestnetAddress() {
			assertIsValidAddressString(mainnet(), addressVectors()[0].testnet(), false);
		}

		@Test
		void testnetRejectsMainnetAddress() {
			assertIsValidAddressString(testnet(), addressVectors()[0].mainnet(), false);
		}

		@Test
		void rejectsBadLength() {
			// Arrange:
			final String valid = addressVectors()[0].mainnet();

			// Act + Assert: shorter, empty, and longer (JS appends a well-formed base32 char) must all be rejected
			assertIsValidAddressString(mainnet(), valid.substring(0, valid.length() - 1), false);
			assertIsValidAddressString(mainnet(), "", false);
			assertIsValidAddressString(mainnet(), valid + "A", false);
		}

		@Test
		void rejectsInvalidBase32() {
			// Arrange: '!' is not a valid base32 char (length kept valid by replacing the last char)
			final String valid = addressVectors()[0].mainnet();
			final String tampered = valid.substring(0, valid.length() - 1) + "!";

			// Act + Assert:
			assertIsValidAddressString(mainnet(), tampered, false);
		}

		@Test
		void rejectsTamperedChecksum() {
			// Arrange: flip a bit in the last (checksum) byte and re-encode. Tampering the encoded string's last character is NOT
			// equivalent for symbol addresses: a 39-char string carries 195 bits for 24 bytes, so the last character's low bits fall
			// into the dropped padding byte and a flip within 'A'..'H' can leave the decoded address (and checksum) unchanged.
			final TAddress valid = addressFromString(addressVectors()[0].mainnet());
			final byte[] tamperedBytes = valid.bytes().clone();
			tamperedBytes[tamperedBytes.length - 1] ^= 0x01;
			final String tampered = addressFromBytes(tamperedBytes).toString();

			// Act + Assert:
			assertIsValidAddressString(mainnet(), tampered, false);
		}
	}

	@Nested
	class IsValidAddress {
		private void assertAcceptsValidAndRejectsTamperedBytes(final Network<TAddress, TNetworkTimestamp> network,
				final String addressString) {
			// Arrange:
			final TAddress valid = addressFromString(addressString);
			final byte[] beginTampered = valid.bytes().clone();
			beginTampered[1] ^= 0xFF;
			final byte[] endTampered = valid.bytes().clone();
			endTampered[endTampered.length - 1] ^= 0xFF;

			// Act:
			final boolean isValidAccepted = network.isValidAddress(valid);
			final boolean isBeginTamperedAccepted = network.isValidAddress(addressFromBytes(beginTampered));
			final boolean isEndTamperedAccepted = network.isValidAddress(addressFromBytes(endTampered));

			// Assert: a valid address passes; tampering the first hash byte or the last checksum byte fails
			assertThat(isValidAccepted, is(true));
			assertThat(isBeginTamperedAccepted, is(false));
			assertThat(isEndTamperedAccepted, is(false));
		}

		@Test
		void mainnetAcceptsValidAndRejectsTamperedBytes() {
			assertAcceptsValidAndRejectsTamperedBytes(mainnet(), addressVectors()[0].mainnet());
		}

		@Test
		void testnetAcceptsValidAndRejectsTamperedBytes() {
			assertAcceptsValidAndRejectsTamperedBytes(testnet(), addressVectors()[0].testnet());
		}
	}

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
}
