package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;

import java.time.Instant;
import java.util.List;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.AbstractNetworkTest;

/**
 * Tests {@link Network} and {@link NetworkTimestamp} against vectors from {@code tests/vectors/nem/crypto/1.test-address.json} and the JS
 * {@code Network_spec}. Shared address/locator contract tests are inherited from {@link AbstractNetworkTest}; NEM's second resolution and
 * predefined networks are covered here, while {@link Address} specifics live in {@link AddressTest}.
 */
final class NetworkTest extends AbstractNetworkTest<Address, NetworkTimestamp> {
	// First three entries from tests/vectors/nem/crypto/1.test-address.json, plus the deterministic public key pinned by the JS
	// shared network tests.
	private static final AddressVector[] ADDRESS_VECTORS = {
			new AddressVector("C5F54BA980FCBB657DBAAA42700539B207873E134D2375EFEAB5F1AB52F87844",
					"NDD2CT6LQLIYQ56KIXI3ENTM6EK3D44P5JFXJ4R4", "TDD2CT6LQLIYQ56KIXI3ENTM6EK3D44P5KZPFMK2"),
			new AddressVector("96EB2A145211B1B7AB5F0D4B14F8ABC8D695C7AEE31A3CFC2D4881313C68EEA3",
					"NABHFGE5ORQD3LE4O6B7JUFN47ECOFBFASC3SCAC", "TABHFGE5ORQD3LE4O6B7JUFN47ECOFBFATE53N2I"),
			new AddressVector("2D8425E4CA2D8926346C7A7CA39826ACD881A8639E81BD68820409C6E30D142A",
					"NAVOZX4HDVOAR4W6K4WJHWPD3MOFU27DFHC7KZOZ", "TAVOZX4HDVOAR4W6K4WJHWPD3MOFU27DFEJDR2PR"),
			new AddressVector("D6C3845431236C5A5A907A9E45BD60DA0E12EFD350B970E7F58E3499E2E7A2F0",
					"NCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNJABUMH", "TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33"),
	};

	// NEM epoch (shared by both networks); the datetime converter uses a second time unit.
	private static final Instant EPOCH = Instant.parse("2015-03-29T00:06:25Z");

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
		void canAddSeconds() {
			// Arrange:
			final NetworkTimestamp timestamp = new NetworkTimestamp(100L);

			// Act:
			final NetworkTimestamp newTimestamp = timestamp.addSeconds(50L);

			// Assert:
			assertThat(timestamp.timestamp, is(100L));
			assertThat(newTimestamp.timestamp, is(100L + 50L));
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

			// Assert: NEM timestamps have second resolution
			assertThat(datetime, equalTo(EPOCH.plusSeconds(123)));
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
			final NetworkTimestamp networkTimestamp = Network.MAINNET.fromDatetime(EPOCH.plusSeconds(123));

			// Assert:
			assertThat(networkTimestamp.isEpochal(), is(false));
			assertThat(networkTimestamp.timestamp, is(123L));
		}
	}

	@Test
	void registersCorrectPredefinedNetworks() {
		// Act: the datetime converter is private in Java, so observe each network's epoch and second time unit through toDatetime
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
		assertThat(mainnetEpoch, equalTo(Instant.parse("2015-03-29T00:06:25Z")));
		assertThat(mainnetEpochPlusOneUnit, equalTo(Instant.parse("2015-03-29T00:06:26Z")));

		assertThat(Network.TESTNET.name, equalTo("testnet"));
		assertThat(Network.TESTNET.identifier, is((byte) 0x98));
		assertThat(testnetEpoch, equalTo(Instant.parse("2015-03-29T00:06:25Z")));
		assertThat(testnetEpochPlusOneUnit, equalTo(Instant.parse("2015-03-29T00:06:26Z")));
	}
}
