package org.symbol.sdk.symbol;

/**
 * Represents a Symbol network timestamp with millisecond resolution.
 */
public final class NetworkTimestamp extends org.symbol.sdk.NetworkTimestamp.Base {
	/**
	 * Creates a Symbol network timestamp.
	 *
	 * @param timestamp Raw network timestamp.
	 */
	public NetworkTimestamp(final long timestamp) {
		super(timestamp);
	}

	/**
	 * Adds a specified number of milliseconds to this timestamp.
	 *
	 * @param count Number of milliseconds to add.
	 * @return New timestamp that is the specified number of milliseconds past this timestamp.
	 */
	public NetworkTimestamp addMilliseconds(final long count) {
		return new NetworkTimestamp(timestamp + count);
	}

	@Override
	public NetworkTimestamp addSeconds(final long count) {
		return addMilliseconds(1000L * count);
	}
}
