package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;

import java.util.Arrays;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class TransformsTest {
	// region sha3_256

	@Nested
	final class Sha3_256 {
		@Test
		void canHashTwoEmptyBuffers() {
			// Arrange:
			final byte[] first = new byte[0];
			final byte[] second = new byte[0];

			// Act:
			final byte[] digest = Transforms.sha3_256(first, second);

			// Assert:
			assertThat(digest.length, equalTo(32));
		}

		@Test
		void canHashTwoNonEmptyBuffers() {
			// Arrange:
			final byte[] first = {
					0x00, 0x01, 0x02
			};
			final byte[] second = {
					0x03, 0x04, 0x05
			};

			// Act:
			final byte[] digest = Transforms.sha3_256(first, second);

			// Assert:
			assertThat(digest.length, equalTo(32));
			// Hash should be deterministic
			final byte[] digest2 = Transforms.sha3_256(first, second);
			assertThat(digest, equalTo(digest2));
		}

		@Test
		void producesConsistentHashForSameInput() {
			// Arrange:
			final byte[] payload1 = Converter.hexToUint8("AABBCC");
			final byte[] payload2 = Converter.hexToUint8("DDEEFF");

			// Act:
			final byte[] digest1 = Transforms.sha3_256(payload1, payload2);
			final byte[] digest2 = Transforms.sha3_256(payload1, payload2);

			// Assert:
			assertThat(digest1, equalTo(digest2));
		}

		@Test
		void differentInputsProduceDifferentHashes() {
			// Arrange:
			final byte[] payload1 = {
					0x11
			};
			final byte[] payload2 = {
					0x22
			};
			final byte[] payload3 = {
					0x33
			};

			// Act:
			final byte[] digest1 = Transforms.sha3_256(payload1, payload2);
			final byte[] digest2 = Transforms.sha3_256(payload1, payload3);

			// Assert:
			// Very unlikely to be equal (SHA3 is cryptographically secure)
			boolean allEqual = Arrays.equals(digest1, digest2);
			assertThat(allEqual, equalTo(false));
		}

		@Test
		void orderMatters() {
			// Arrange:
			final byte[] payload1 = {
					0x11
			};
			final byte[] payload2 = {
					0x22
			};

			// Act:
			final byte[] digest1 = Transforms.sha3_256(payload1, payload2);
			final byte[] digest2 = Transforms.sha3_256(payload2, payload1);

			// Assert:
			boolean allEqual = Arrays.equals(digest1, digest2);
			assertThat(allEqual, equalTo(false));
		}

		@Test
		void fitsLargePayloads() {
			// Arrange:
			final byte[] first = new byte[1000];
			final byte[] second = new byte[1000];
			java.util.Arrays.fill(first, (byte) 0xAA);
			java.util.Arrays.fill(second, (byte) 0xBB);

			// Act:
			final byte[] digest = Transforms.sha3_256(first, second);

			// Assert:
			assertThat(digest.length, equalTo(32));
		}

		@Test
		void preservesFirstBufferContent() {
			// Arrange:
			final byte[] first = {
					0x01, 0x02, 0x03
			};
			final byte[] firstOriginal = Arrays.copyOf(first, first.length);
			final byte[] second = {
					0x04, 0x05, 0x06
			};

			// Act:
			Transforms.sha3_256(first, second);

			// Assert:
			assertThat(first, equalTo(firstOriginal));
		}

		@Test
		void preservesSecondBufferContent() {
			// Arrange:
			final byte[] first = {
					0x01, 0x02, 0x03
			};
			final byte[] second = {
					0x04, 0x05, 0x06
			};
			final byte[] secondOriginal = Arrays.copyOf(second, second.length);

			// Act:
			Transforms.sha3_256(first, second);

			// Assert:
			assertThat(second, equalTo(secondOriginal));
		}
	}

	// endregion

	// region sha3_256 varargs

	@Nested
	final class Sha3_256Varargs {
		@Test
		void canHashSingleBuffer() {
			// Arrange:
			final byte[] payload = {
					0x01, 0x02, 0x03
			};

			// Act:
			final byte[] digest = Transforms.sha3_256(payload);

			// Assert:
			assertThat(digest.length, equalTo(32));
		}

		@Test
		void canHashThreeBuffers() {
			// Arrange:
			final byte[] payload1 = {
					0x01
			};
			final byte[] payload2 = {
					0x02
			};
			final byte[] payload3 = {
					0x03
			};

			// Act:
			final byte[] digest = Transforms.sha3_256(payload1, payload2, payload3);

			// Assert:
			assertThat(digest.length, equalTo(32));
		}

		@Test
		void canHashManyBuffers() {
			// Arrange:
			final byte[][] payloads = new byte[10][];
			for (int i = 0; i < payloads.length; ++i) {
				payloads[i] = new byte[]{
						(byte) i
				};
			}

			// Act:
			final byte[] digest = Transforms.sha3_256(payloads);

			// Assert:
			assertThat(digest.length, equalTo(32));
		}

		@Test
		void producesConsistentHashForSameInput() {
			// Arrange:
			final byte[] payload1 = {
					0x01, 0x02
			};
			final byte[] payload2 = {
					0x03, 0x04
			};
			final byte[] payload3 = {
					0x05, 0x06
			};

			// Act:
			final byte[] digest1 = Transforms.sha3_256(payload1, payload2, payload3);
			final byte[] digest2 = Transforms.sha3_256(payload1, payload2, payload3);

			// Assert:
			assertThat(digest1, equalTo(digest2));
		}

		@Test
		void emptyBuffersArrayProducesHash() {
			// Arrange:
			final byte[][] payloads = new byte[0][];

			// Act:
			final byte[] digest = Transforms.sha3_256(payloads);

			// Assert:
			assertThat(digest.length, equalTo(32));
		}

		@Test
		void twoBufferVarargsEquivalentToTwoBufferOverload() {
			// Arrange:
			final byte[] payload1 = Converter.hexToUint8("AABBCC");
			final byte[] payload2 = Converter.hexToUint8("DDEEFF");

			// Act:
			final byte[] digest1 = Transforms.sha3_256(payload1, payload2);
			final byte[] digest2 = Transforms.sha3_256(new byte[][]{
					payload1, payload2
			});

			// Assert:
			assertThat(digest1, equalTo(digest2));
		}
	}

	// endregion

	// region ripemdKeccak256

	@Nested
	final class RipemdKeccak256 {
		@Test
		void canTransformWithRipemdKeccak256() {
			// Arrange:
			final byte[] payload = Converter.hexToUint8("BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F");

			// Act:
			final byte[] hashResult = Transforms.ripemdKeccak256(payload);

			// Assert:
			assertThat(hashResult, equalTo(Converter.hexToUint8("FDB8D529F3656230A7FD6F183A0E8D750E4033C3")));
		}

		@Test
		void producesConsistentHashForSameInput() {
			// Arrange:
			final byte[] payload = Converter.hexToUint8("BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F");

			// Act:
			final byte[] hash1 = Transforms.ripemdKeccak256(payload);
			final byte[] hash2 = Transforms.ripemdKeccak256(payload);

			// Assert:
			assertThat(hash1, equalTo(hash2));
		}

		@Test
		void producesHash20Bytes() {
			// Arrange:
			final byte[] payload = new byte[]{
					0x01, 0x02, 0x03
			};

			// Act:
			final byte[] hash = Transforms.ripemdKeccak256(payload);

			// Assert:
			assertThat(hash.length, equalTo(20));
		}

		@Test
		void canHashEmptyBuffer() {
			// Arrange:
			final byte[] payload = new byte[0];

			// Act:
			final byte[] hash = Transforms.ripemdKeccak256(payload);

			// Assert:
			assertThat(hash.length, equalTo(20));
		}

		@Test
		void differentInputsProduceDifferentHashes() {
			// Arrange:
			final byte[] payload1 = {
					0x11
			};
			final byte[] payload2 = {
					0x22
			};

			// Act:
			final byte[] hash1 = Transforms.ripemdKeccak256(payload1);
			final byte[] hash2 = Transforms.ripemdKeccak256(payload2);

			// Assert:
			boolean allEqual = Arrays.equals(hash1, hash2);
			assertThat(allEqual, equalTo(false));
		}

		@Test
		void doesNotModifyInputPayload() {
			// Arrange:
			final byte[] payload = {
					0x01, 0x02, 0x03, 0x04, 0x05
			};
			final byte[] payloadOriginal = Arrays.copyOf(payload, payload.length);

			// Act:
			Transforms.ripemdKeccak256(payload);

			// Assert:
			assertThat(payload, equalTo(payloadOriginal));
		}

		@Test
		void canHashLargePayload() {
			// Arrange:
			final byte[] payload = new byte[10000];
			java.util.Arrays.fill(payload, (byte) 0xFF);

			// Act:
			final byte[] hash = Transforms.ripemdKeccak256(payload);

			// Assert:
			assertThat(hash.length, equalTo(20));
		}
	}

	// endregion
}
