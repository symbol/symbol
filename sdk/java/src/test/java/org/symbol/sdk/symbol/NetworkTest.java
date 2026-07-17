package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

import java.time.Instant;
import java.util.List;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.AbstractNetworkTest;
import org.symbol.sdk.CryptoTypes;

/**
 * Tests {@link Network} and {@link NetworkTimestamp} against vectors from {@code tests/vectors/symbol/crypto/1.test-address.json} and the
 * JS {@code Network_spec}. Shared address/locator contract tests are inherited from {@link AbstractNetworkTest}; Symbol's millisecond
 * resolution and predefined networks are covered here, while {@link Address} specifics (aliasing, namespace ids) live in
 * {@link AddressTest}.
 */
final class NetworkTest extends AbstractNetworkTest<Address, NetworkTimestamp> {
	// First three entries from tests/vectors/symbol/crypto/1.test-address.json, plus the deterministic public key pinned by the JS
	// shared network tests.
	private static final AddressVector[] ADDRESS_VECTORS = {
			new AddressVector("2E834140FD66CF87B254A693A2C7862C819217B676D3943267156625E816EC6F", "NATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA34SQ33Y",
					"TATNE7Q5BITMUTRRN6IB4I7FLSDRDWZA37JGO5Q"),
			new AddressVector("4875FD2E32875D1BC6567745F1509F0F890A1BF8EE59FA74452FA4183A270E03", "NDR6EW2WBHJQDYMNGFX2UBZHMMZC5PGL2YCZOQQ",
					"TDR6EW2WBHJQDYMNGFX2UBZHMMZC5PGL2YBO3KA"),
			new AddressVector("9F780097FB6A1F287ED2736A597B8EA7F08D20F1ECDB9935DE6694ECF1C58900", "NCOXVZMAZJTT4I3F7EAZYGNGR77D6WPTRH6SYIQ",
					"TCOXVZMAZJTT4I3F7EAZYGNGR77D6WPTRE3VIBQ"),
			new AddressVector("C5FB65CB902623D93DF2E682FFB13F99D50FAC24D5FF2A42F68C7CA1772FE8A0", "NBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNKZIBEY",
					"TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA"),
	};

	// Symbol mainnet epoch; the datetime converter uses a millisecond time unit.
	private static final Instant EPOCH = Instant.parse("2021-03-16T00:06:25Z");

	@Override
	protected Network mainnet() {
		return Network.MAINNET;
	}

	@Override
	protected Network testnet() {
		return Network.TESTNET;
	}

	@Override
	protected List<Network> networks() {
		return Network.NETWORKS;
	}

	@Override
	protected AddressVector[] addressVectors() {
		return ADDRESS_VECTORS;
	}

	@Override
	protected Address addressFromString(final String addressString) {
		return new Address(addressString);
	}

	@Override
	protected Address addressFromBytes(final byte[] addressBytes) {
		return new Address(addressBytes);
	}

	@Nested
	class NetworkTimestampTest {
		@Test
		void canAddMilliseconds() {
			// Arrange:
			final NetworkTimestamp timestamp = new NetworkTimestamp(100L);

			// Act:
			final NetworkTimestamp newTimestamp = timestamp.addMilliseconds(50L);

			// Assert:
			assertThat(timestamp.timestamp, is(100L));
			assertThat(newTimestamp.timestamp, is(100L + 50L));
		}

		@Test
		void canAddSeconds() {
			// Arrange:
			final NetworkTimestamp timestamp = new NetworkTimestamp(100L);

			// Act:
			final NetworkTimestamp newTimestamp = timestamp.addSeconds(50L);

			// Assert:
			assertThat(timestamp.timestamp, is(100L));
			assertThat(newTimestamp.timestamp, is(100L + (50L * 1000L)));
		}
	}

	@Nested
	class ToDatetime {
		@Test
		void canConvertEpochalTimestampToDatetime() {
			// Act:
			final Instant datetime = Network.MAINNET.toDatetime(new NetworkTimestamp(0L));

			// Assert:
			assertThat(datetime, equalTo(EPOCH));
		}

		@Test
		void canConvertNonEpochalTimestampToDatetime() {
			// Act:
			final Instant datetime = Network.MAINNET.toDatetime(new NetworkTimestamp(123L));

			// Assert: symbol timestamps have millisecond resolution
			assertThat(datetime, equalTo(EPOCH.plusMillis(123)));
		}
	}

	@Nested
	class FromDatetime {
		@Test
		void canConvertDatetimeToEpochalTimestamp() {
			// Act:
			final NetworkTimestamp networkTimestamp = Network.MAINNET.fromDatetime(EPOCH);

			// Assert:
			assertThat(networkTimestamp.isEpochal(), is(true));
			assertThat(networkTimestamp.timestamp, is(0L));
		}

		@Test
		void canConvertDatetimeToNonEpochalTimestamp() {
			// Act:
			final NetworkTimestamp networkTimestamp = Network.MAINNET.fromDatetime(EPOCH.plusMillis(123));

			// Assert:
			assertThat(networkTimestamp.isEpochal(), is(false));
			assertThat(networkTimestamp.timestamp, is(123L));
		}
	}

	@Test
	void registersCorrectPredefinedNetworks() {
		// Act: the datetime converter is private in Java, so observe each network's epoch and millisecond time unit through toDatetime
		final List<String> names = Network.NETWORKS.stream().map(network -> network.name).toList();
		final Instant mainnetEpoch = Network.MAINNET.toDatetime(new NetworkTimestamp(0L));
		final Instant mainnetEpochPlusOneUnit = Network.MAINNET.toDatetime(new NetworkTimestamp(1L));
		final Instant testnetEpoch = Network.TESTNET.toDatetime(new NetworkTimestamp(0L));
		final Instant testnetEpochPlusOneUnit = Network.TESTNET.toDatetime(new NetworkTimestamp(1L));

		// Assert:
		assertThat(Network.NETWORKS.size(), is(2));
		assertThat(names, equalTo(List.of("mainnet", "testnet")));

		assertThat(Network.MAINNET.name, equalTo("mainnet"));
		assertThat(Network.MAINNET.identifier, is((byte) 0x68));
		assertThat(mainnetEpoch, equalTo(Instant.parse("2021-03-16T00:06:25Z")));
		assertThat(mainnetEpochPlusOneUnit, equalTo(Instant.parse("2021-03-16T00:06:25.001Z")));
		assertThat(Network.MAINNET.generationHashSeed,
				equalTo(new CryptoTypes.Hash256("57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6")));

		assertThat(Network.TESTNET.name, equalTo("testnet"));
		assertThat(Network.TESTNET.identifier, is((byte) 0x98));
		assertThat(testnetEpoch, equalTo(Instant.parse("2022-10-31T21:07:47Z")));
		assertThat(testnetEpochPlusOneUnit, equalTo(Instant.parse("2022-10-31T21:07:47.001Z")));
		assertThat(Network.TESTNET.generationHashSeed,
				equalTo(new CryptoTypes.Hash256("49D6E1CE276A85B70EAFE52349AACCA389302E7A9754BCF1221E79494FC665A4")));
	}
}
