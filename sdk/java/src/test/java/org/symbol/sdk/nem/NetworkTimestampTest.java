package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.NetworkTimestamp.Base;
import org.symbol.sdk.nem.models.*;

/**
 * Tests {@link NetworkTimestamp}. NEM uses second resolution, so {@code addSeconds} advances 1:1 (unlike the Symbol subclass, which
 * multiplies by 1000 because it uses millisecond resolution).
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
		assertThat(new NetworkTimestamp(0L).isEpochal(), is(true));
	}

	@Test
	void canAddSeconds() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(100L);

		// Act:
		final Base result = ts.addSeconds(50L);

		// Assert:
		assertThat(result, instanceOf(NetworkTimestamp.class));
		assertThat(result.timestamp, equalTo(150L));
		// original is unchanged (immutability contract)
		assertThat(ts.timestamp, equalTo(100L));
	}

	@Test
	void canAddMinutes() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(0L);

		// Act:
		final Base result = ts.addMinutes(2L);

		// Assert:
		assertThat(result, instanceOf(NetworkTimestamp.class));
		assertThat(result.timestamp, equalTo(2L * 60L));
	}

	@Test
	void canAddHours() {
		// Arrange:
		final NetworkTimestamp ts = new NetworkTimestamp(0L);

		// Act:
		final Base result = ts.addHours(3L);

		// Assert:
		assertThat(result, instanceOf(NetworkTimestamp.class));
		assertThat(result.timestamp, equalTo(3L * 60L * 60L));
	}

	@Test
	void equalsAndHashCodeMatchUnderlyingTimestamp() {
		// Arrange:
		final NetworkTimestamp a = new NetworkTimestamp(42L);
		final NetworkTimestamp b = new NetworkTimestamp(42L);
		final NetworkTimestamp c = new NetworkTimestamp(43L);

		// Act + Assert:
		assertThat(a, equalTo(b));
		assertThat(a.hashCode(), equalTo(b.hashCode()));
		assertThat(a.equals(c), is(false));
	}
}
