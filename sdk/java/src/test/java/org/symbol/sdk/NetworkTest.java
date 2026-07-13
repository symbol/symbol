package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.time.Instant;
import java.util.List;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class NetworkTest {
	private static final class FakeTimestamp extends NetworkTimestamp.Base {
		FakeTimestamp(final long timestamp) {
			super(timestamp);
		}

		@Override
		public NetworkTimestamp.Base addSeconds(final long count) {
			return new FakeTimestamp(timestamp + count);
		}
	}

	private static final class FakeAddress extends ByteArray {
		FakeAddress(final byte[] bytes) {
			super(bytes, bytes.length);
		}
	}

	private static Network<FakeAddress, FakeTimestamp> createNetwork(final String name, final byte identifier) {
		final NetworkTimestamp.NetworkTimestampDatetimeConverter converter = new NetworkTimestamp.NetworkTimestampDatetimeConverter(
				Instant.parse("2022-03-16T00:06:25Z"), NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit.MINUTES);
		return new Network<>(name, identifier, converter, () -> null, (version, checksum) -> null, 0, addressString -> null,
				raw -> new FakeTimestamp(raw));
	}

	// region Network

	@Nested
	final class NetworkBase {
		@Test
		void canConvertNetworkTimeToDatetime() {
			// Arrange:
			final Network<FakeAddress, FakeTimestamp> network = createNetwork("foo", (byte) 0x55);

			// Act:
			final Instant datetimeTimestamp = network.toDatetime(new FakeTimestamp(60));

			// Assert:
			assertThat(datetimeTimestamp, equalTo(Instant.parse("2022-03-16T01:06:25Z")));
		}

		@Test
		void canConvertDatetimeToNetworkTime() {
			// Arrange:
			final Network<FakeAddress, FakeTimestamp> network = createNetwork("foo", (byte) 0x55);

			// Act:
			final FakeTimestamp networkTimestamp = network.fromDatetime(Instant.parse("2022-03-16T01:06:25Z"));

			// Assert:
			assertThat(networkTimestamp.timestamp, equalTo(60L));
		}

		@Test
		void supportsToString() {
			// Arrange:
			final Network<FakeAddress, FakeTimestamp> network = createNetwork("foo", (byte) 0x55);

			// Act + Assert:
			assertThat(network.toString(), equalTo("foo"));
		}
	}

	// endregion

	// region NetworkLocator

	@Nested
	final class NetworkLocatorTests {
		private final List<Network<FakeAddress, FakeTimestamp>> predefinedNetworks = List.of(createNetwork("foo", (byte) 0x55),
				createNetwork("bar", (byte) 0x37));

		@Test
		void canFindWellKnownNetworkByNameSingle() {
			assertThat(Network.NetworkLocator.findByName(predefinedNetworks, "foo"), sameInstance(predefinedNetworks.get(0)));
			assertThat(Network.NetworkLocator.findByName(predefinedNetworks, "bar"), sameInstance(predefinedNetworks.get(1)));
		}

		@Test
		void canFindWellKnownNetworkByNameList() {
			assertThat(Network.NetworkLocator.findByName(predefinedNetworks, List.of("xxx", "foo")),
					sameInstance(predefinedNetworks.get(0)));
			assertThat(Network.NetworkLocator.findByName(predefinedNetworks, List.of("bar", "yyy")),
					sameInstance(predefinedNetworks.get(1)));
			assertThat(Network.NetworkLocator.findByName(predefinedNetworks, List.of("bar", "foo")),
					sameInstance(predefinedNetworks.get(0)));
		}

		@Test
		void cannotFindOtherNetworkByName() {
			assertThrows(IllegalArgumentException.class, () -> Network.NetworkLocator.findByName(predefinedNetworks, "cat"));
			assertThrows(IllegalArgumentException.class,
					() -> Network.NetworkLocator.findByName(predefinedNetworks, List.of("cat", "dog")));
		}

		@Test
		void canFindWellKnownNetworkByIdentifierSingle() {
			assertThat(Network.NetworkLocator.findByIdentifier(predefinedNetworks, (byte) 0x55), sameInstance(predefinedNetworks.get(0)));
			assertThat(Network.NetworkLocator.findByIdentifier(predefinedNetworks, (byte) 0x37), sameInstance(predefinedNetworks.get(1)));
		}

		@Test
		void canFindWellKnownNetworkByIdentifierList() {
			assertThat(Network.NetworkLocator.findByIdentifier(predefinedNetworks, List.of((byte) 0x88, (byte) 0x55)),
					sameInstance(predefinedNetworks.get(0)));
			assertThat(Network.NetworkLocator.findByIdentifier(predefinedNetworks, List.of((byte) 0x37, (byte) 0x99)),
					sameInstance(predefinedNetworks.get(1)));
			assertThat(Network.NetworkLocator.findByIdentifier(predefinedNetworks, List.of((byte) 0x37, (byte) 0x55)),
					sameInstance(predefinedNetworks.get(0)));
		}

		@Test
		void cannotFindOtherNetworkByIdentifier() {
			assertThrows(IllegalArgumentException.class, () -> Network.NetworkLocator.findByIdentifier(predefinedNetworks, (byte) 0xFF));
			assertThrows(IllegalArgumentException.class,
					() -> Network.NetworkLocator.findByIdentifier(predefinedNetworks, List.of((byte) 0xFF, (byte) 0x88)));
		}
	}

	// endregion
}
