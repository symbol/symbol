package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.time.Instant;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class NetworkTimestampTest {
	private static final class FakeTimestamp extends NetworkTimestamp.Base {
		FakeTimestamp(final long timestamp) {
			super(timestamp);
		}

		@Override
		public NetworkTimestamp.Base addSeconds(final long count) {
			return new FakeTimestamp(timestamp + count);
		}
	}

	@Nested
	final class TimestampBase {
		@Test
		void epochalTimestampIsRecognized() {
			// Act + Assert:
			assertThat(new FakeTimestamp(0).isEpochal(), is(true));
			assertThat(new FakeTimestamp(1).isEpochal(), is(false));
		}

		@Test
		void addSecondsAdvancesByCorrectAmount() {
			// Arrange:
			final FakeTimestamp t = new FakeTimestamp(100);

			// Act + Assert:
			assertThat(((FakeTimestamp) t.addSeconds(50)).timestamp, equalTo((long) (150)));
		}

		@Test
		void canAddMinutes() {
			// Arrange:
			final FakeTimestamp t = new FakeTimestamp(100);

			// Act + Assert:
			assertThat(((FakeTimestamp) t.addMinutes(2)).timestamp, equalTo((long) (100 + 2 * 60)));
		}

		@Test
		void canAddHours() {
			// Arrange:
			final FakeTimestamp t = new FakeTimestamp(0);

			// Act + Assert:
			assertThat(((FakeTimestamp) t.addHours(3)).timestamp, equalTo((long) (3L * 60 * 60)));
		}

		@Test
		void supportsToString() {
			// Act:
			final String actual = new FakeTimestamp(123456789L).toString();

			// Assert:
			assertThat(actual, equalTo("123456789"));
		}

		@Test
		void toStringRendersRawTimestampAsUnsigned() {
			// Arrange: the raw timestamp is a u64 — values >= 2^63 must render unsigned, matching BaseValue and the JS BigInt reference
			final FakeTimestamp t = new FakeTimestamp(Long.MIN_VALUE);

			// Act:
			final String actual = t.toString();

			// Assert:
			assertThat(actual, equalTo("9223372036854775808"));
		}

		@Test
		void equalsIsReflexive() {
			// Arrange:
			final FakeTimestamp a = new FakeTimestamp(100);

			// Act + Assert:
			assertEquality(a, a, true);
		}

		@Test
		void equalsRejectsNonBaseType() {
			// Arrange:
			final FakeTimestamp a = new FakeTimestamp(100);

			// Act + Assert:
			assertEquality(a, "not a timestamp", false);
		}

		@Test
		void equalsTrueForSameTimestamp() {
			// Arrange:
			final FakeTimestamp a = new FakeTimestamp(100);

			// Act + Assert:
			assertEquality(a, new FakeTimestamp(100), true);
		}

		@Test
		void equalsFalseForDifferentTimestamp() {
			// Arrange:
			final FakeTimestamp a = new FakeTimestamp(100);

			// Act + Assert:
			assertEquality(a, new FakeTimestamp(101), false);
		}
	}

	private static void assertEquality(final Object value, final Object other, final boolean expected) {
		// Act:
		final boolean actual = value.equals(other);

		// Assert:
		assertThat(actual, equalTo(expected));
	}

	@Nested
	final class DatetimeConverter {
		@Test
		void canConvertNetworkTimestampToDatetimeInSeconds() {
			// Arrange:
			final Instant epoch = Instant.parse("2020-01-01T00:00:00Z");
			final NetworkTimestamp.NetworkTimestampDatetimeConverter conv = new NetworkTimestamp.NetworkTimestampDatetimeConverter(epoch,
					NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit.SECONDS);

			// Act:
			final Instant result = conv.toDatetime(120);

			// Assert:
			assertThat(result, equalTo(epoch.plusSeconds(120)));
		}

		@Test
		void canConvertDatetimeToDifferenceInSeconds() {
			// Arrange:
			final Instant epoch = Instant.parse("2020-01-01T00:00:00Z");
			final NetworkTimestamp.NetworkTimestampDatetimeConverter conv = new NetworkTimestamp.NetworkTimestampDatetimeConverter(epoch,
					NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit.SECONDS);

			// Act:
			final long diff = conv.toDifference(epoch.plusSeconds(120));

			// Assert:
			assertThat(diff, equalTo(120L));
		}

		@Test
		void cannotConvertDatetimeBeforeEpochalTimestamp() {
			// Arrange:
			final Instant epoch = Instant.parse("2020-01-01T00:00:00Z");
			final NetworkTimestamp.NetworkTimestampDatetimeConverter conv = new NetworkTimestamp.NetworkTimestampDatetimeConverter(epoch,
					NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit.SECONDS);

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
					() -> conv.toDifference(epoch.minusSeconds(1)));

			// Assert:
			assertThat(ex.getMessage(), containsString("before epoch"));
		}
	}
}
