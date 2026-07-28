package org.symbol.sdk.symbol;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.not;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

import org.symbol.sdk.symbol.models.*;

/** Tests {@link Restriction}. */
final class RestrictionTest {
	@Nested
	class MosaicRestrictionGenerateKey {
		@Test
		void canGenerateExpectedKeysFromSeeds() {
			// Act:
			final long keyA = Restriction.mosaicRestrictionGenerateKey("a");
			final long keyAbc = Restriction.mosaicRestrictionGenerateKey("abc");
			final long keyDef = Restriction.mosaicRestrictionGenerateKey("def");

			// Assert:
			assertThat(keyA, equalTo(0x7524A0FBF24B0880L));
			assertThat(keyAbc, equalTo(0xB225E24FA75D983AL));
			assertThat(keyDef, equalTo(0xB0AC5222678F0D8EL));
		}

		@Test
		void isDeterministic() {
			// Act:
			final long key1 = Restriction.mosaicRestrictionGenerateKey("hello");
			final long key2 = Restriction.mosaicRestrictionGenerateKey("hello");

			// Assert:
			assertThat(key1, equalTo(key2));
		}

		@Test
		void isDifferentForDifferentSeeds() {
			// Act:
			final long fooKey = Restriction.mosaicRestrictionGenerateKey("foo");
			final long barKey = Restriction.mosaicRestrictionGenerateKey("bar");

			// Assert:
			assertThat(fooKey, not(equalTo(barKey)));
		}
	}
}
