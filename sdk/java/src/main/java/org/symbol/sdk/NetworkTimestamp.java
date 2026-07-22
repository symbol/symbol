package org.symbol.sdk;

import java.time.Instant;

/**
 * Network timestamp types ({@link Base} and {@link NetworkTimestampDatetimeConverter}).
 */
public final class NetworkTimestamp {
	private NetworkTimestamp() {
	}

	/**
	 * Represents a network timestamp. Concrete subclasses must implement {@link #addSeconds(long)}.
	 */
	public abstract static class Base {
		/**
		 * Underlying timestamp.
		 */
		public final long timestamp;

		/**
		 * Creates a timestamp.
		 *
		 * @param timestamp Raw network timestamp.
		 */
		public Base(final long timestamp) {
			this.timestamp = timestamp;
		}

		/**
		 * Determines if this is the epochal timestamp.
		 *
		 * @return {@code true} if this is the epochal timestamp.
		 */
		public boolean isEpochal() {
			return 0L == timestamp;
		}

		/**
		 * Adds a specified number of seconds to this timestamp.
		 *
		 * @param count Number of seconds to add.
		 * @return New timestamp that is the specified number of seconds past this timestamp.
		 */
		public abstract Base addSeconds(long count);

		/**
		 * Adds a specified number of minutes to this timestamp.
		 *
		 * @param count Number of minutes to add.
		 * @return New timestamp that is the specified number of minutes past this timestamp.
		 */
		public Base addMinutes(final long count) {
			return addSeconds(60L * count);
		}

		/**
		 * Adds a specified number of hours to this timestamp.
		 *
		 * @param count Number of hours to add.
		 * @return New timestamp that is the specified number of hours past this timestamp.
		 */
		public Base addHours(final long count) {
			return addMinutes(60L * count);
		}

		@Override
		public String toString() {
			return Long.toUnsignedString(timestamp);
		}

		@Override
		public boolean equals(final Object other) {
			if (this == other)
				return true;

			if (null == other || getClass() != other.getClass())
				return false;

			return timestamp == ((Base) other).timestamp;
		}

		@Override
		public int hashCode() {
			return java.util.Objects.hash(getClass(), timestamp);
		}
	}

	/**
	 * Provides utilities for converting between network timestamps and datetimes.
	 */
	public static final class NetworkTimestampDatetimeConverter {
		/**
		 * Time units understood by this converter.
		 */
		public enum TimeUnit {
			HOURS(60L * 60 * 1000),
			MINUTES(60L * 1000),
			SECONDS(1000L),
			MILLISECONDS(1L);

			private final long millis;

			TimeUnit(final long millis) {
				this.millis = millis;
			}
		}

		/**
		 * Date at which network started.
		 */
		public final Instant epoch;

		/**
		 * Number of milliseconds per time unit.
		 */
		public final long timeUnits;

		/**
		 * Creates a converter given an epoch and base time units.
		 *
		 * @param epoch Date at which network started.
		 * @param timeUnits Time unit the network uses for progressing.
		 */
		public NetworkTimestampDatetimeConverter(final Instant epoch, final TimeUnit timeUnits) {
			this.epoch = epoch;
			this.timeUnits = timeUnits.millis;
		}

		/**
		 * Converts a network timestamp to a datetime.
		 *
		 * @param rawTimestamp Raw network timestamp.
		 * @return Datetime representation of the network timestamp.
		 */
		public Instant toDatetime(final long rawTimestamp) {
			return epoch.plusMillis(rawTimestamp * timeUnits);
		}

		/**
		 * Subtracts the network epoch from the reference date.
		 *
		 * @param referenceDatetime Reference date.
		 * @return Number of network time units between the reference date and the network epoch.
		 */
		public long toDifference(final Instant referenceDatetime) {
			if (referenceDatetime.isBefore(epoch))
				throw new IllegalArgumentException("timestamp cannot be before epoch");

			final long differenceMilliseconds = referenceDatetime.toEpochMilli() - epoch.toEpochMilli();
			return differenceMilliseconds / timeUnits;
		}
	}
}
