package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.anEmptyMap;
import static org.hamcrest.Matchers.contains;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.nullValue;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.charset.StandardCharsets;
import java.util.List;

import org.junit.jupiter.api.Test;

import org.symbol.sdk.utils.Writer;

/**
 * Tests the {@link CatbufferType} base behaviours that generated structs inherit but do not exercise: the default {@code toJson}/{@code
 * setField}/{@code getField}/{@code sort}/{@code typeHints} surface and the {@code asXxx} descriptor-conversion helpers (including their
 * reject paths).
 */
final class CatbufferTypeTest {
	/** Minimal concrete struct: a 4-byte body, no descriptor fields, and the inherited default {@code toJson}. */
	private static final class Fixture extends CatbufferType {
		@Override
		public int size() {
			return 4;
		}

		@Override
		protected void serializeInto(final Writer buffer) {
			buffer.writeInt(0x04030201, 4);
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

	@Test
	void serializeSizesWriterAndThreadsSerializeInto() {
		// Act + Assert:
		assertThat(new Fixture().serialize(), equalTo(new byte[]{
				0x01, 0x02, 0x03, 0x04
		}));
	}

	@Test
	void toJsonThrowsByDefault() {
		// Act + Assert:
		assertThrows(UnsupportedOperationException.class, () -> new Fixture().toJson());
	}

	@Test
	void toJsonStringSerializesTheProjection() {
		// Act + Assert:
		assertThat(new JsonFixture(java.util.Map.of("k", "v")).toJsonString(), equalTo("{\"k\":\"v\"}"));
	}

	@Test
	void toJsonStringWrapsSerializationFailureAsIllegalState() {
		// Arrange:
		final java.util.Map<String, Object> selfReferential = new java.util.HashMap<>();
		selfReferential.put("self", selfReferential); // Jackson rejects the direct cycle with a JsonProcessingException

		// Act + Assert:
		assertThrows(IllegalStateException.class, () -> new JsonFixture(selfReferential).toJsonString());
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
		// Act + Assert:
		assertThat(CatbufferType.asList(null, String.class), nullValue());
		assertThat(CatbufferType.asList(List.of("a", "b"), String.class), contains("a", "b"));
		assertThrows(ClassCastException.class, () -> CatbufferType.asList(List.of(1, 2), String.class));
	}

	@Test
	void asBytesAcceptsBytesStringByteArrayAndRejectsOthers() {
		// Arrange:
		final byte[] raw = {
				1, 2, 3
		};

		// Act + Assert:
		assertThat(CatbufferType.asBytes(raw), sameInstance(raw));
		assertThat(CatbufferType.asBytes("AB"), equalTo("AB".getBytes(StandardCharsets.UTF_8)));
		final CryptoTypes.Hash256 hash = CryptoTypes.Hash256.zero();
		assertThat(CatbufferType.asBytes(hash), equalTo(hash.bytes()));
		assertThat(CatbufferType.asBytes(null), nullValue());
		assertThrows(InvalidDescriptorException.class, () -> CatbufferType.asBytes(42));
	}

	@Test
	void asByteArrayShortCircuitsRewrapsAndRejectsOthers() {
		// Arrange:
		final CryptoTypes.Hash256 hash = new CryptoTypes.Hash256("AB".repeat(32));

		// Act + Assert:
		// already the target type -> returned as-is
		assertThat(CatbufferType.asByteArray(hash, CryptoTypes.Hash256.class, CryptoTypes.Hash256::new), sameInstance(hash));
		// a different ByteArray with matching bytes -> rewrapped via the ctor
		final CryptoTypes.PublicKey key = new CryptoTypes.PublicKey("AB".repeat(32));
		assertThat(CatbufferType.asByteArray(key, CryptoTypes.Hash256.class, CryptoTypes.Hash256::new), equalTo(hash));
		// raw bytes -> wrapped via the ctor
		assertThat(CatbufferType.asByteArray(hash.bytes(), CryptoTypes.Hash256.class, CryptoTypes.Hash256::new), equalTo(hash));
		assertThat(CatbufferType.asByteArray(null, CryptoTypes.Hash256.class, CryptoTypes.Hash256::new), nullValue());
		assertThrows(InvalidDescriptorException.class,
				() -> CatbufferType.asByteArray("nope", CryptoTypes.Hash256.class, CryptoTypes.Hash256::new));
	}

	@Test
	void numericConvertersAcceptNumbersAndRejectOthers() {
		// Act + Assert:
		assertThat(CatbufferType.asInt(300), equalTo(300));
		assertThat(CatbufferType.asLong(300L), equalTo(300L));
		assertThat(CatbufferType.asShort(300), equalTo((short) 300));
		assertThat(CatbufferType.asByte(300), equalTo((byte) 300));
		assertThrows(InvalidDescriptorException.class, () -> CatbufferType.asInt("x"));
		assertThrows(InvalidDescriptorException.class, () -> CatbufferType.asLong("x"));
		assertThrows(InvalidDescriptorException.class, () -> CatbufferType.asShort("x"));
		assertThrows(InvalidDescriptorException.class, () -> CatbufferType.asByte("x"));
	}
}
