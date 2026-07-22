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
			return new FakeTimestamp(timestamp + 5 * count);
		}
	}

	@Nested
	final class TimestampBase {
		@Test
		void canCreateEpochalTimestamp() {
			// Act:
			final FakeTimestamp timestamp = new FakeTimestamp(0);

			// Assert:
			assertThat(timestamp.isEpochal(), is(true));
			assertThat(timestamp.timestamp, equalTo(0L));
		}

		@Test
		void canCreateNonEpochalTimestamp() {
			// Act:
			final FakeTimestamp timestamp = new FakeTimestamp(123);

			// Assert:
			assertThat(timestamp.isEpochal(), is(false));
			assertThat(timestamp.timestamp, equalTo(123L));
		}

		@Test
		void canAddMinutes() {
			// Arrange:
			final FakeTimestamp timestamp = new FakeTimestamp(100);

			// Act:
			final NetworkTimestamp.Base newTimestamp = timestamp.addMinutes(50);

			// Assert: the original is unchanged; the new value went through the subclass addSeconds (x5)
			assertThat(timestamp.timestamp, equalTo(100L));
			assertThat(newTimestamp.timestamp, equalTo(100 + 60L * 5 * 50));
		}

		@Test
		void canAddHours() {
			// Arrange:
			final FakeTimestamp timestamp = new FakeTimestamp(100);

			// Act:
			final NetworkTimestamp.Base newTimestamp = timestamp.addHours(50);

			// Assert: the original is unchanged; the new value went through the subclass addSeconds (x5)
			assertThat(timestamp.timestamp, equalTo(100L));
			assertThat(newTimestamp.timestamp, equalTo(100 + 60L * 60 * 5 * 50));
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

			// Assert:
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

		@Test
		void hashCodeMatchesUnderlyingTimestamp() {
			// Act:
			final int hashA = new FakeTimestamp(100).hashCode();
			final int hashB = new FakeTimestamp(100).hashCode();

			// Assert:
			assertThat(hashA, equalTo(hashB));
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
		// the JS spec's Date.UTC(2020, 1, 2, 3) — an hours-resolution converter epoch of 2020-02-02T03:00Z
		private static final Instant EPOCH = Instant.parse("2020-02-02T03:00:00Z");

		private NetworkTimestamp.NetworkTimestampDatetimeConverter createConverter() {
			return new NetworkTimestamp.NetworkTimestampDatetimeConverter(EPOCH,
					NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit.HOURS);
		}

		@Test
		void canConvertEpochalTimestampToDatetime() {
			// Act:
			final Instant result = createConverter().toDatetime(0);

			// Assert:
			assertThat(result, equalTo(EPOCH));
		}

		@Test
		void canConvertNonEpochalTimestampToDatetime() {
			// Act:
			final Instant result = createConverter().toDatetime(5);

			// Assert:
			assertThat(result, equalTo(EPOCH.plus(5, java.time.temporal.ChronoUnit.HOURS)));
		}

		@Test
		void cannotConvertDatetimeBeforeEpochalTimestamp() {
			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
					() -> createConverter().toDifference(EPOCH.minus(1, java.time.temporal.ChronoUnit.HOURS)));

			// Assert:
			assertThat(ex.getMessage(), containsString("before epoch"));
		}

		@Test
		void canConvertDatetimeToEpochalTimestamp() {
			// Act: four minutes past the epoch truncates to zero whole hours
			final long rawTimestamp = createConverter().toDifference(EPOCH.plus(4, java.time.temporal.ChronoUnit.MINUTES));

			// Assert:
			assertThat(rawTimestamp, equalTo(0L));
		}

		@Test
		void canConvertDatetimeToNonEpochalTimestamp() {
			// Act: five hours and four minutes past the epoch truncates to five whole hours
			final long rawTimestamp = createConverter()
					.toDifference(EPOCH.plus(5, java.time.temporal.ChronoUnit.HOURS).plus(4, java.time.temporal.ChronoUnit.MINUTES));

			// Assert:
			assertThat(rawTimestamp, equalTo(5L));
		}

		@Test
		void canConvertDatetimeToNonEpochalTimestampLarge() {
			// Arrange: a milliseconds-resolution difference spanning five years ((5 * 365) + 2 leap days), which
			// exceeds 2^31 — guarding against any int-width arithmetic in the conversion
			final NetworkTimestamp.NetworkTimestampDatetimeConverter converter = new NetworkTimestamp.NetworkTimestampDatetimeConverter(
					EPOCH, NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit.MILLISECONDS);

			// Act:
			final long rawTimestamp = converter.toDifference(Instant.parse("2025-02-02T03:00:00Z"));

			// Assert:
			assertThat(rawTimestamp, equalTo(((5L * 365) + 2) * 24 * 60 * 60 * 1000));
			assertThat(rawTimestamp > (1L << 31), is(true));
		}
	}
}
