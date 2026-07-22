package org.symbol.sdk.nem;

import java.time.Instant;
import java.util.List;

import org.bouncycastle.crypto.digests.KeccakDigest;

import org.symbol.sdk.NetworkTimestamp.NetworkTimestampDatetimeConverter;
import org.symbol.sdk.NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit;
import org.symbol.sdk.utils.ArrayHelpers;

/**
 * Represents a NEM network.
 */
public final class Network extends org.symbol.sdk.Network<Address, NetworkTimestamp> {
	private static final Instant NEM_EPOCH = Instant.parse("2015-03-29T00:06:25Z");

	/** NEM main network. */
	public static final Network MAINNET = new Network("mainnet", (byte) 0x68, NEM_EPOCH);

	/** NEM test network. */
	public static final Network TESTNET = new Network("testnet", (byte) 0x98, NEM_EPOCH);

	/** NEM well-known networks. */
	public static final List<Network> NETWORKS = List.of(MAINNET, TESTNET);

	/**
	 * Creates a new NEM network.
	 *
	 * @param name Network name.
	 * @param identifier Network identifier byte.
	 * @param epochTime Network epoch time.
	 */
	public Network(final String name, final byte identifier, final Instant epochTime) {
		super(name, identifier, new NetworkTimestampDatetimeConverter(epochTime, TimeUnit.SECONDS), () -> new KeccakDigest(256),
				// NEM addresses use the full 4-byte checksum the base derives
				(addressWithoutChecksum, checksum) -> new Address(ArrayHelpers.concat(addressWithoutChecksum, checksum)),
				Address.ENCODED_SIZE, Address::new, timestamp -> new NetworkTimestamp((long) timestamp));
	}
}
