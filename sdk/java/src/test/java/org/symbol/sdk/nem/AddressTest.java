package org.symbol.sdk.nem;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.utils.Converter;

/**
 * Tests {@link Address}. Mirrors the {@code Address (NEM)} tests in {@code test/nem/Network_spec.js} (via {@code runBasicAddressTests}) and
 * adds Java-only coverage for {@link Address#parse(Object)}, the default constructor, and {@link Address#deserialize(byte[])}.
 */
final class AddressTest {

	// Encoded/decoded pair pinned by test/nem/Network_spec.js.
	private static final String ENCODED_ADDRESS = "TCFGSLITSWMRROU2GO7FPMIUUDELUPSZUNUEZF33";
	private static final byte[] DECODED_ADDRESS = Converter.hexToUint8("988A692D13959918BA9A33BE57B114A0C8BA3E59A3684C977B");

	@Test
	void hasCorrectSize() {
		// Assert:
		assertThat(Address.SIZE, is(DECODED_ADDRESS.length));
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
		// (Java-only) the copy constructor must not alias the source
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
	void hasCorrectConstants() {
		// Assert: (JS also pins Address.NAME, which has no Java counterpart)
		assertThat(Address.SIZE, is(25));
		assertThat(Address.ENCODED_SIZE, is(40));
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
		assertThat(fromAddress, not(sameInstance(original))); // parse defensively copies, like the symbol Address
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
	}

	@Test
	void constructorRejectsWrongSizedBytes() { // (Java-only)
		// Act + Assert:
		assertThrows(IllegalArgumentException.class, () -> new Address(new byte[Address.SIZE - 1]));
		assertThrows(IllegalArgumentException.class, () -> new Address(new byte[Address.SIZE + 1]));
	}

	@Test
	void deserializeReadsExactlySizeBytes() { // (Java-only)
		// Arrange: payload with trailing garbage that must be ignored.
		final byte[] payload = new byte[Address.SIZE + 8];
		System.arraycopy(DECODED_ADDRESS, 0, payload, 0, Address.SIZE);
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
}
