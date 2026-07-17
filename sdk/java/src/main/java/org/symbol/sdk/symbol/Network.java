package org.symbol.sdk.symbol;

import java.time.Instant;
import java.util.Arrays;
import java.util.List;

import org.bouncycastle.crypto.digests.SHA3Digest;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.NetworkTimestamp.NetworkTimestampDatetimeConverter;
import org.symbol.sdk.NetworkTimestamp.NetworkTimestampDatetimeConverter.TimeUnit;
import org.symbol.sdk.utils.ArrayHelpers;

/**
 * Represents a Symbol network.
 */
public final class Network extends org.symbol.sdk.Network<Address, NetworkTimestamp> {
	/** Symbol main network. */
	public static final Network MAINNET = new Network("mainnet", (byte) 0x68, Instant.parse("2021-03-16T00:06:25Z"),
			new CryptoTypes.Hash256("57F7DA205008026C776CB6AED843393F04CD458E0AA2D9F1D5F31A402072B2D6"));

	/** Symbol test network. */
	public static final Network TESTNET = new Network("testnet", (byte) 0x98, Instant.parse("2022-10-31T21:07:47Z"),
			new CryptoTypes.Hash256("49D6E1CE276A85B70EAFE52349AACCA389302E7A9754BCF1221E79494FC665A4"));

	/** Symbol well-known networks. */
	public static final List<Network> NETWORKS = List.of(MAINNET, TESTNET);

	/** Network generation hash seed. */
	public final CryptoTypes.Hash256 generationHashSeed;

	/**
	 * Creates a new Symbol network.
	 *
	 * @param name Network name.
	 * @param identifier Network identifier byte.
	 * @param epochTime Network epoch time.
	 * @param generationHashSeed Network generation hash seed.
	 */
	public Network(final String name, final byte identifier, final Instant epochTime, final CryptoTypes.Hash256 generationHashSeed) {
		super(name, identifier, new NetworkTimestampDatetimeConverter(epochTime, TimeUnit.MILLISECONDS), () -> new SHA3Digest(256),
				// Symbol addresses use a 3-byte checksum (the base derives a 4-byte one, so truncate)
				(addressWithoutChecksum, checksum) -> new Address(ArrayHelpers.concat(addressWithoutChecksum, Arrays.copyOf(checksum, 3))),
				Address.ENCODED_SIZE, Address::new, timestamp -> new NetworkTimestamp((long) timestamp));
		this.generationHashSeed = generationHashSeed;
	}
}
