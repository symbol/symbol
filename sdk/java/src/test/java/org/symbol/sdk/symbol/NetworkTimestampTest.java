package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.NetworkTimestamp.Base;
import org.symbol.sdk.symbol.models.*;

/**
 * Tests {@link NetworkTimestamp}. Symbol uses millisecond resolution, so {@code addSeconds} multiplies its argument by 1000 (unlike the NEM
 * subclass, which uses second resolution).
 */
final class NetworkTimestampTest {

	@Test
	void canCreateNonEpochalTimestamp() {
		// Act:
		final NetworkTimestamp ts = new NetworkTimestamp(123L);

		// Assert:
		assertThat(ts.timestamp, equalTo(123L));
		assertThat(ts.isEpochal(), is(false));
	}

	@Test
	void canCreateEpochalTimestamp() {
		// Act:
		final boolean actual = new NetworkTimestamp(0L).isEpochal();

		// Assert:
		assertThat(actual, is(true));
	}

	@Test
	void canAddMilliseconds() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(100L);

		// Act:
		final NetworkTimestamp result = ts.addMilliseconds(50L);

		// Assert:
		assertThat(result.timestamp, equalTo(150L));
		// original is unchanged (immutability contract)
		assertThat(ts.timestamp, equalTo(100L));
	}

	@Test
	void canAddSeconds() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(100L);

		// Act:
		final Base result = ts.addSeconds(50L);

		// Assert:
		assertThat(result, instanceOf(NetworkTimestamp.class));
		assertThat(result.timestamp, equalTo(100L + 50L * 1000L));
	}

	@Test
	void canAddMinutes() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(0L);

		// Act:
		final Base result = ts.addMinutes(2L);

		// Assert:
		assertThat(result, instanceOf(NetworkTimestamp.class));
		assertThat(result.timestamp, equalTo(2L * 60L * 1000L));
	}

	@Test
	void canAddHours() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(0L);

		// Act:
		final Base result = ts.addHours(3L);

		// Assert:
		assertThat(result, instanceOf(NetworkTimestamp.class));
		assertThat(result.timestamp, equalTo(3L * 60L * 60L * 1000L));
	}

	@Test
	void equalsAndHashCodeMatchUnderlyingTimestamp() {
		// Arrange:
		final NetworkTimestamp a = new NetworkTimestamp(42L);
		final NetworkTimestamp b = new NetworkTimestamp(42L);
		final NetworkTimestamp c = new NetworkTimestamp(43L);

		// Assert:
		assertThat(a, equalTo(b));
		assertThat(a.hashCode(), equalTo(b.hashCode()));
		assertThat(a.equals(c), is(false));
	}
}
