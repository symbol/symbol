package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.Test;

/** Tests {@link JsonDescriptor}. */
final class JsonDescriptorTest {
	@Test
	void parsesIntegralNumbers() {
		// Act:
		final Map<String, Object> descriptor = JsonDescriptor.parse("{\"amount\": 1000000, \"type\": \"transfer\"}");

		// Assert: the wrapper type varies by magnitude (Integer/Long/BigInteger); the descriptor coercions accept all forms,
		// so only the numeric value matters.
		assertThat(((Number) descriptor.get("amount")).longValue(), equalTo(1_000_000L));
		assertThat(descriptor.get("type"), equalTo("transfer"));
	}

	@Test
	void parsesBareU64AboveSignedLongRangeAsBigInteger() {
		// Act: a bare u64 >= 2^63 (e.g. any metadata scopedMetadataKey) exceeds a signed long and deserializes as BigInteger,
		// which the u64 descriptor coercion accepts as a bit pattern.
		final Map<String, Object> descriptor = JsonDescriptor.parse("{\"mosaicId\": 18446744073709551615}");

		// Assert:
		assertThat(descriptor.get("mosaicId"), equalTo(new java.math.BigInteger("18446744073709551615")));
	}

	@Test
	void preservesLargeU64SuppliedAsString() {
		// Act:
		final Map<String, Object> descriptor = JsonDescriptor.parse("{\"mosaicId\": \"0xFFFFFFFFFFFFFFFF\"}");

		// Assert: the string passes through unchanged for the pod parser to coerce via Long.parseUnsignedLong.
		assertThat(descriptor.get("mosaicId"), equalTo("0xFFFFFFFFFFFFFFFF"));
	}

	@Test
	void parsesNestedObjectsAndArrays() {
		// Act:
		final Map<String, Object> descriptor = JsonDescriptor
				.parse("{\"mosaics\": [{\"id\": 1, \"amount\": 2}], \"message\": {\"text\": \"hi\"}}");

		// Assert:
		assertThat(descriptor.get("mosaics"), instanceOf(List.class));
		final List<?> mosaics = (List<?>) descriptor.get("mosaics");
		assertThat(((Number) ((Map<?, ?>) mosaics.get(0)).get("amount")).longValue(), equalTo(2L));
		assertThat(((Map<?, ?>) descriptor.get("message")).get("text"), equalTo("hi"));
	}

	@Test
	void rejectsMalformedJson() {
		assertThrows(InvalidDescriptorException.class, () -> JsonDescriptor.parse("{not valid json"));
	}

	@Test
	void rejectsNonObjectTopLevel() {
		assertThrows(InvalidDescriptorException.class, () -> JsonDescriptor.parse("[1, 2, 3]"));
	}

	@Test
	void rejectsJsonNullLiteral() {
		// the literal `null` deserializes to a null map; reject it rather than returning null (which NPEs downstream)
		assertThrows(InvalidDescriptorException.class, () -> JsonDescriptor.parse("null"));
	}

	@Test
	void rejectsTrailingTokensAfterObject() {
		// content after the top-level object must be rejected (matching JSON.parse / json.loads), not silently dropped
		assertThrows(InvalidDescriptorException.class, () -> JsonDescriptor.parse("{\"a\": 1} garbage"));
		assertThrows(InvalidDescriptorException.class, () -> JsonDescriptor.parse("{\"a\": 1} {\"b\": 2}"));
	}
}
