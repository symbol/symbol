package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.nullValue;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link Address} against the JS {@code Network_spec} ('Address (Symbol)' describe, including the shared basic address tests), plus
 * Java-only surface: {@link Address#parse(Object)}, {@link Address#deserialize(byte[])}, and default/copy constructor behavior.
 */
final class AddressTest {
	// Vectors pinned from the JS Network_spec 'Address (Symbol)' describe.
	private static final String ENCODED_ADDRESS = "TBLYH55IHPS5QCCMNWR3GZWKV6WMCKPTNI7KSDA";
	private static final byte[] DECODED_ADDRESS = Converter.hexToUint8("985783F7A83BE5D8084C6DA3B366CAAFACC129F36A3EA90C");

	// region JS parity tests

	@Test
	void hasCorrectSize() {
		// Assert:
		assertThat(Address.SIZE, is(DECODED_ADDRESS.length));
	}

	@Test
	void hasCorrectConstants() {
		// Assert: (JS also pins Address.NAME, which has no Java counterpart)
		assertThat(Address.SIZE, is(24));
		assertThat(Address.ENCODED_SIZE, is(39));
	}

	@Test
	void canCreateFromAddress() {
		// Arrange:
		final Address originalAddress = new Address(ENCODED_ADDRESS);

		// Act:
		final Address address = new Address(originalAddress);

		// Assert:
		assertThat(address.bytes(), is(equalTo(DECODED_ADDRESS)));
		assertThat(address.toString(), is(equalTo(ENCODED_ADDRESS)));
		// (Java-only) the copy must be deep
		assertThat(address, is(not(sameInstance(originalAddress))));
		assertThat(address.bytes(), is(not(sameInstance(originalAddress.bytes()))));
	}

	@Test
	void canCreateFromEncodedAddress() {
		// Act:
		final Address address = new Address(ENCODED_ADDRESS);

		// Assert:
		assertThat(address.bytes(), is(equalTo(DECODED_ADDRESS)));
		assertThat(address.toString(), is(equalTo(ENCODED_ADDRESS)));
	}

	@Test
	void canCreateFromDecodedAddress() {
		// Act:
		final Address address = new Address(DECODED_ADDRESS);

		// Assert:
		assertThat(address.bytes(), is(equalTo(DECODED_ADDRESS)));
		assertThat(address.toString(), is(equalTo(ENCODED_ADDRESS)));
	}

	@Test
	void cannotExtractNamespaceIdFromNonAliasAddress() {
		// Arrange:
		final Address address = new Address(ENCODED_ADDRESS);

		// Act:
		final NamespaceId namespaceId = address.toNamespaceId();

		// Assert: (JS returns undefined; Java returns null)
		assertThat(namespaceId, is(nullValue()));
	}

	@Test
	void canExtractNamespaceIdFromAliasAddress() {
		// Arrange:
		final Address address = new Address("THBIMC3THGH5RUYAAAAAAAAAAAAAAAAAAAAAAAA");

		// Act:
		final NamespaceId namespaceId = address.toNamespaceId();

		// Assert:
		assertThat(namespaceId, is(equalTo(new NamespaceId(0xD3D88F39730B86C2L))));
	}

	@Test
	void canBeCreatedFromDecodedAddressHexString() {
		// Act:
		final Address address = Address.fromDecodedAddressHexString("980E356BFE40284E4C9C532CB2D5260F6D5FC029D35D2D62");

		// Assert:
		assertThat(address.toString(), is(equalTo("TAHDK276IAUE4TE4KMWLFVJGB5WV7QBJ2NOS2YQ")));
	}

	@Test
	void canBeCreatedFromNamespaceId() {
		// Act: network identifier 152 == 0x98 (testnet); the alias flag is networkIdentifier + 1
		final Address address = Address.fromNamespaceId(new NamespaceId(0xD3D88F39730B86C2L), (byte) 152);

		// Assert:
		assertThat(address.toString(), is(equalTo("THBIMC3THGH5RUYAAAAAAAAAAAAAAAAAAAAAAAA")));
		assertThat(address.isAlias(), is(true)); // (Java-only assertion)
	}

	@Test
	void canDetectAlias() {
		// Act:
		final boolean isAliasWithBitUnset1 = new Address("TAHDK276IAUE4TE4KMWLFVJGB5WV7QBJ2NOS2YQ").isAlias();
		final boolean isAliasWithBitUnset2 = new Address(ENCODED_ADDRESS).isAlias();
		final boolean isAliasWithBitSet = new Address("THBIMC3THGH5RUYAAAAAAAAAAAAAAAAAAAAAAAA").isAlias();

		// Assert: 8th bit unset => false
		assertThat(isAliasWithBitUnset1, is(false));
		assertThat(isAliasWithBitUnset2, is(false));

		// - 8th bit set => true
		assertThat(isAliasWithBitSet, is(true));
	}

	// endregion

	// region Java-only tests

	@Test
	void canDetectAliasFromRawBytes() { // (Java-only) independent byte-level oracle for canDetectAlias
		// Arrange: the low bit of the first byte selects alias-ness (0x69 = testnet alias byte, 0x68 = testnet regular byte)
		final byte[] aliasBytes = new byte[Address.SIZE];
		aliasBytes[0] = 0x69;
		final byte[] regularBytes = new byte[Address.SIZE];
		regularBytes[0] = 0x68;

		// Act:
		final boolean isAliasDetected = new Address(aliasBytes).isAlias();
		final boolean isRegularDetected = new Address(regularBytes).isAlias();

		// Assert:
		assertThat(isAliasDetected, is(true));
		assertThat(isRegularDetected, is(false));
	}

	@Test
	void decodedHexRoundTripsThroughEncodedForm() { // (Java-only) independent oracle for canBeCreatedFromDecodedAddressHexString
		// Arrange: derive the hex form from the encoded form rather than pinning it
		final String hexString = Converter.uint8ToHex(new Address(ENCODED_ADDRESS).bytes());

		// Act:
		final Address address = Address.fromDecodedAddressHexString(hexString);

		// Assert:
		assertThat(address, is(equalTo(new Address(ENCODED_ADDRESS))));
		assertThat(address.toString(), is(equalTo(ENCODED_ADDRESS)));
	}

	@Test
	void parseAcceptsExistingAddressStringAndRawBytes() { // (Java-only)
		// Arrange:
		final Address original = new Address(ENCODED_ADDRESS);

		// Act:
		final Address fromAddress = Address.parse(original);
		final Address fromString = Address.parse(ENCODED_ADDRESS);
		final Address fromBytes = Address.parse(original.bytes());

		// Assert:
		assertThat(fromAddress, equalTo(original));
		assertThat(fromAddress, not(sameInstance(original))); // symbol parse defensively copies
		assertThat(fromString, equalTo(original));
		assertThat(fromBytes, equalTo(original));
	}

	@Test
	void defaultConstructorProducesZeroBytes() { // (Java-only)
		// Act:
		final Address address = new Address();

		// Assert:
		assertThat(address.bytes().length, is(Address.SIZE));
		for (final byte b : address.bytes())
			assertThat(b, is((byte) 0));
		assertThat(address.isAlias(), is(false));
	}

	@Test
	void constructorRejectsWrongSizedBytes() { // (Java-only)
		// Act + Assert:
		assertThrows(IllegalArgumentException.class, () -> new Address(new byte[Address.SIZE - 1]));
		assertThrows(IllegalArgumentException.class, () -> new Address(new byte[Address.SIZE + 1]));
	}

	@Test
	void deserializeReadsExactlySizeBytes() { // (Java-only)
		// Arrange: payload with trailing garbage that must be ignored
		final byte[] payload = new byte[Address.SIZE + 8];
		System.arraycopy(DECODED_ADDRESS, 0, payload, 0, DECODED_ADDRESS.length);
		for (int i = Address.SIZE; i < payload.length; ++i)
			payload[i] = (byte) 0xAB;

		// Act:
		final Address address = Address.deserialize(payload);

		// Assert:
		assertThat(address.bytes(), is(equalTo(DECODED_ADDRESS)));
		assertThat(address.toString(), is(equalTo(ENCODED_ADDRESS)));
	}

	@Test
	void deserializeRejectsTruncatedPayload() { // (Java-only)
		// Act + Assert: a short payload must throw, not be zero-padded into a valid-looking address
		assertThrows(IndexOutOfBoundsException.class, () -> Address.deserialize(new byte[Address.SIZE - 1]));
		assertThrows(IndexOutOfBoundsException.class, () -> Address.deserialize(new byte[0]));
		// an offset that leaves fewer than SIZE bytes must throw too
		assertThrows(IndexOutOfBoundsException.class, () -> Address.deserialize(new byte[Address.SIZE], 1));
	}

	@Test
	void toJsonMatchesToString() { // (Java-only)
		// Arrange:
		final Address address = new Address(ENCODED_ADDRESS);

		// Act:
		final String json = address.toJson();

		// Assert:
		assertThat(json, is(equalTo(address.toString())));
		assertThat(json, is(equalTo(ENCODED_ADDRESS)));
	}

	// endregion
}
