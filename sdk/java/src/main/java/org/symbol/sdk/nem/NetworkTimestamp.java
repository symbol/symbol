package org.symbol.sdk.nem;

/**
 * Represents a NEM network timestamp with second resolution.
 */
public final class NetworkTimestamp extends org.symbol.sdk.NetworkTimestamp.Base {
	/**
	 * Creates a NEM network timestamp.
	 *
	 * @param timestamp Raw network timestamp.
	 */
	public NetworkTimestamp(final long timestamp) {
		super(timestamp);
	}

	@Override
	public NetworkTimestamp addSeconds(final long count) {
		return new NetworkTimestamp(timestamp + count);
	}
}
