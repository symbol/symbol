package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.charset.StandardCharsets;
import java.util.List;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.utils.Writer;

/**
 * Tests the {@link CatbufferType} base behaviours that generated structs inherit but do not exercise: the default {@code setField}/{@code
 * getField}/{@code sort}/{@code typeHints} surface, {@code toJsonString} over a supplied projection, and the {@code asXxx}
 * descriptor-conversion helpers (including their reject paths).
 */
final class CatbufferTypeTest {
	/** Minimal concrete struct: a 4-byte body and no descriptor fields. */
	private static final class Fixture extends CatbufferType {
		@Override
		public int size() {
			return 4;
		}

		@Override
		protected void serializeInto(final Writer buffer) {
			buffer.writeInt(0x04030201, 4);
		}

		@Override
		public Object toJson() {
			return java.util.Map.of();
		}
	}

	/** Struct whose {@code toJson} returns a caller-supplied projection, to exercise {@link CatbufferType#toJsonString()}. */
	private static final class JsonFixture extends CatbufferType {
		private final Object json;

		JsonFixture(final Object json) {
			this.json = json;
		}

		@Override
		public int size() {
			return 0;
		}

		@Override
		protected void serializeInto(final Writer buffer) {
		}

		@Override
		public Object toJson() {
			return json;
		}
	}

	/** Struct with one named field whose {@code setField}/{@code getField} mirror the generated switch-and-chain-to-super pattern. */
	private static final class FieldFixture extends CatbufferType {
		private int value;

		@Override
		public int size() {
			return 4;
		}

		@Override
		protected void serializeInto(final Writer buffer) {
			buffer.writeInt(value, 4);
		}

		@Override
		public Object toJson() {
			return java.util.Map.of("value", value);
		}

		@Override
		public void setField(final String name, final Object fieldValue) {
			switch (name) {
				case "value" -> this.value = asInt(fieldValue);
				default -> super.setField(name, fieldValue);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "value" -> this.value;
				default -> super.getField(name);
			};
		}
	}

	@Test
	void serializeWriterAndSerializeInto() {
		// Act:
		final byte[] actual = new Fixture().serialize();

		// Assert:
		assertThat(actual, equalTo(new byte[]{
				0x01, 0x02, 0x03, 0x04
		}));
	}

	@Test
	void toJsonStringSerializesTheProjection() {
		// Act:
		final String actual = new JsonFixture(java.util.Map.of("k", "v")).toJsonString();

		// Assert:
		assertThat(actual, equalTo("{\"k\":\"v\"}"));
	}

	@Test
	void toJsonStringWrapsSerializationFailureAsIllegalStateException() {
		// Arrange: a projection that is not JSON-serializable
		final Object projection = java.util.Map.of("bad", new Fixture());

		// Act + Assert:
		final IllegalStateException ex = assertThrows(IllegalStateException.class, () -> new JsonFixture(projection).toJsonString());
		assertThat(ex.getMessage(), equalTo("toJson() of " + JsonFixture.class.getName() + " is not JSON-serializable"));
		assertThat(ex.getCause(), instanceOf(com.fasterxml.jackson.core.JsonProcessingException.class));
	}

	@Test
	void overriddenSetFieldCoercesAndGetFieldReadsBack() {
		// Arrange:
		final FieldFixture fixture = new FieldFixture();

		// Act: a descriptor string is coerced by asInt on the way in, then read back as the typed value
		fixture.setField("value", "300");

		// Assert:
		assertThat(fixture.getField("value"), equalTo(300));
	}

	@Test
	void overriddenSetAndGetFieldChainToSuperForUnknownField() {
		// Arrange:
		final FieldFixture fixture = new FieldFixture();

		// Act + Assert: an unknown name falls through the switch default to the throwing base
		assertThrows(InvalidDescriptorException.class, () -> fixture.setField("nope", 1));
		assertThrows(InvalidDescriptorException.class, () -> fixture.getField("nope"));
	}

	@Test
	void setAndGetFieldThrowForUnknownFieldByDefault() {
		// Arrange:
		final Fixture fixture = new Fixture();

		// Act + Assert:
		assertThrows(InvalidDescriptorException.class, () -> fixture.setField("nope", 1));
		assertThrows(InvalidDescriptorException.class, () -> fixture.getField("nope"));
	}

	@Test
	void sortHaveInertDefaults() {
		// Arrange:
		final Fixture fixture = new Fixture();

		// Act + Assert:
		assertDoesNotThrow(fixture::sort);
	}

	@Test
	void typeHintsHaveInertDefaults() {
		// Arrange:
		final Fixture fixture = new Fixture();

		// Act + Assert:
		assertThat(fixture.typeHints(), anEmptyMap());
	}

	@Test
	void asListChecksElementsAndPassesNullThrough() {
		assertThat(CatbufferType.asList(null, String.class), nullValue());
		assertThat(CatbufferType.asList(List.of("a", "b"), String.class), contains("a", "b"));
		assertThrows(ClassCastException.class, () -> CatbufferType.asList(List.of(1, 2), String.class));
	}

	@Test
	void asBytesAcceptsBytesStringAndByteArrayPod() {
		// Arrange:
		final byte[] raw = {
				1, 2, 3
		};
		final CryptoTypes.Hash256 hash = CryptoTypes.Hash256.zero();

		// Act + Assert: byte[] returned as-is, String UTF-8 encoded, ByteArray pod unwrapped to its bytes
		assertThat(CatbufferType.asBytes(raw), sameInstance(raw));
		assertThat(CatbufferType.asBytes("AB"), equalTo("AB".getBytes(StandardCharsets.UTF_8)));
		assertThat(CatbufferType.asBytes(hash), equalTo(hash.bytes()));
	}

	@Test
	void asBytesPassesNullThrough() {
		// Act:
		final byte[] bytes = CatbufferType.asBytes(null);

		// Assert:
		assertThat(bytes, nullValue());
	}

	@Test
	void asBytesRejectsOtherTypes() {
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asBytes(42));
	}

	@Test
	void asByteArrayReturnsSameTypeAsIs() {
		// Arrange:
		final CryptoTypes.Hash256 hash = new CryptoTypes.Hash256("AB".repeat(32));

		// Act: already the target type -> returned as-is
		final CryptoTypes.Hash256 actual = CatbufferType.asByteArray(hash, CryptoTypes.Hash256.class, CryptoTypes.Hash256::new);

		// Assert:
		assertThat(actual, sameInstance(hash));
	}

	@Test
	void asByteArrayRejectsUnrelatedByteArrayType() {
		// Arrange: only a same-named twin bridges (e.g. CryptoTypes.PublicKey -> models.PublicKey); a differently-named ByteArray of equal
		// width is no longer silently reinterpreted, so a PublicKey is rejected for a Hash256 target rather than coerced
		final CryptoTypes.PublicKey key = new CryptoTypes.PublicKey("AB".repeat(32));

		// Act + Assert:
		assertThrows(IllegalArgumentException.class,
				() -> CatbufferType.asByteArray(key, CryptoTypes.Hash256.class, CryptoTypes.Hash256::new));
	}

	@Test
	void asByteArrayWrapsRawBytes() {
		// Arrange:
		final CryptoTypes.Hash256 hash = new CryptoTypes.Hash256("AB".repeat(32));

		// Act: raw bytes -> wrapped via the ctor
		final CryptoTypes.Hash256 actual = CatbufferType.asByteArray(hash.bytes(), CryptoTypes.Hash256.class, CryptoTypes.Hash256::new);

		// Assert:
		assertThat(actual, equalTo(hash));
	}

	@Test
	void asByteArrayPassesNullThrough() {
		// Act:
		final CryptoTypes.Hash256 actual = CatbufferType.asByteArray(null, CryptoTypes.Hash256.class, CryptoTypes.Hash256::new);

		// Assert:
		assertThat(actual, nullValue());
	}

	@Test
	void asByteArrayRejectsOtherTypes() {
		assertThrows(IllegalArgumentException.class,
				() -> CatbufferType.asByteArray("nope", CryptoTypes.Hash256.class, CryptoTypes.Hash256::new));
	}

	private static void assertCoerces(final java.util.function.Function<Object, Object> convert, final Object input,
			final Object expected) {
		// Act:
		final Object actual = convert.apply(input);

		// Assert:
		assertThat(actual, equalTo(expected));
	}

	@Test
	void asIntCoercesNumberAndString() {
		// numbers coerce; asInt also accepts a decimal string.
		assertCoerces(CatbufferType::asInt, 300, 300);
		assertCoerces(CatbufferType::asInt, "300", 300);
	}

	@Test
	void asLongCoercesNumberAndHexString() {
		// numbers coerce; asLong also accepts a 0x-hex string (the canonical u64 >= 2^63 form).
		assertCoerces(CatbufferType::asLong, 300L, 300L);
		assertCoerces(CatbufferType::asLong, "0x12C", 300L);
	}

	@Test
	void numericConvertersRejectNonCoercibleValues() {
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asInt(java.util.List.of()));
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asLong(java.util.List.of()));
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asInt("x"));
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asLong("x"));
	}

	@Test
	void numericConvertersRejectNull() {
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asInt(null));
		assertThrows(IllegalArgumentException.class, () -> CatbufferType.asLong(null));
	}
}
