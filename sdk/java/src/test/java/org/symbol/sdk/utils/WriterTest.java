package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.math.BigInteger;
import java.nio.BufferOverflowException;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class WriterTest {
	// region write

	@Nested
	final class Write {
		@Test
		void cannotWriteIntoEmptyBuffer() {
			// Arrange:
			final Writer writer = new Writer(0);

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> writer.write(new byte[]{
					42
			}));
		}

		@Test
		void canWriteIntoNonEmptyBuffer() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.write(new byte[]{
					42, 3, 14, 15
			});

			// Assert:
			assertThat(writer.offset, equalTo(4));
			assertThat(writer.storage, equalTo(new byte[]{
					42, 3, 14, 15, 0, 0, 0, 0, 0, 0
			}));
		}

		@Test
		void allWritesAreSaved() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.write(new byte[]{
					42, 3, 14
			});
			writer.write(new byte[]{
					15, 92, 65, 35
			});

			// Assert:
			assertThat(writer.offset, equalTo(7));
			assertThat(writer.storage, equalTo(new byte[]{
					42, 3, 14, 15, 92, 65, 35, 0, 0, 0
			}));
		}

		@Test
		void canWriteUntilTheEndOfTheBuffer() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.write(new byte[]{
					42, 3, 14, 15, 92, 65, 35
			});
			writer.write(new byte[]{
					89, 79, 32
			});

			// Assert:
			assertThat(writer.offset, equalTo(10));
			assertThat(writer.storage, equalTo(new byte[]{
					42, 3, 14, 15, 92, 65, 35, 89, 79, 32
			}));
		}

		@Test
		void cannotWritePastTheEndOfTheBuffer() {
			assertCannotWritePastTheEnd(7);
			assertCannotWritePastTheEnd(10);
		}

		@Test
		void emptyByteArrayCanBeWritten() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.write(new byte[]{});

			// Assert:
			assertThat(writer.offset, equalTo(0));
		}

		private void assertCannotWritePastTheEnd(final int initialWriteSize) {
			// Arrange:
			final Writer writer = new Writer(10);
			writer.write(new byte[initialWriteSize]);

			// Sanity:
			assertThat(writer.offset, equalTo(initialWriteSize));

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> writer.write(new byte[]{
					89, 79, 32, 38
			}));
		}
	}

	// endregion

	// region writeInt

	@Nested
	final class WriteInt {
		@Test
		void canWrite1ByteInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(42, 1);

			// Assert:
			assertThat(writer.offset, equalTo(1));
			assertThat(writer.storage[0], equalTo((byte) 42));
		}

		@Test
		void canWrite2ByteIntLittleEndian() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(0x1234, 2);

			// Assert:
			assertThat(writer.offset, equalTo(2));
			// Little-endian: 0x34 0x12
			assertThat(writer.storage[0], equalTo((byte) 0x34));
			assertThat(writer.storage[1], equalTo((byte) 0x12));
		}

		@Test
		void canWrite4ByteIntLittleEndian() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(0x12345678L, 4);

			// Assert:
			assertThat(writer.offset, equalTo(4));
			// Little-endian: 0x78 0x56 0x34 0x12
			assertThat(writer.storage[0], equalTo((byte) 0x78));
			assertThat(writer.storage[1], equalTo((byte) 0x56));
			assertThat(writer.storage[2], equalTo((byte) 0x34));
			assertThat(writer.storage[3], equalTo((byte) 0x12));
		}

		@Test
		void canWriteMultipleInts() {
			// Arrange:
			final Writer writer = new Writer(20);

			// Act:
			writer.writeInt(0xFF, 1);
			writer.writeInt(0x1234, 2);
			writer.writeInt(0x12345678L, 4);

			// Assert:
			assertThat(writer.offset, equalTo(7));
			assertThat(writer.storage[0], equalTo((byte) 0xFF));
			assertThat(writer.storage[1], equalTo((byte) 0x34));
			assertThat(writer.storage[2], equalTo((byte) 0x12));
			assertThat(writer.storage[3], equalTo((byte) 0x78));
			assertThat(writer.storage[4], equalTo((byte) 0x56));
			assertThat(writer.storage[5], equalTo((byte) 0x34));
			assertThat(writer.storage[6], equalTo((byte) 0x12));
		}

		@Test
		void canWrite1ByteNegativeValue() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(-1, 1);

			// Assert:
			assertThat(writer.offset, equalTo(1));
			assertThat(writer.storage[0], equalTo((byte) 0xFF));
		}

		@Test
		void canWrite2ByteNegativeValue() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(-1, 2);

			// Assert:
			assertThat(writer.offset, equalTo(2));
			assertThat(writer.storage[0], equalTo((byte) 0xFF));
			assertThat(writer.storage[1], equalTo((byte) 0xFF));
		}

		@Test
		void canWrite4ByteNegativeValue() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(-1, 4);

			// Assert:
			assertThat(writer.offset, equalTo(4));
			assertThat(writer.storage[0], equalTo((byte) 0xFF));
			assertThat(writer.storage[1], equalTo((byte) 0xFF));
			assertThat(writer.storage[2], equalTo((byte) 0xFF));
			assertThat(writer.storage[3], equalTo((byte) 0xFF));
		}

		@Test
		void cannotWrite3ByteInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeInt(0x123456, 3));
		}

		@Test
		void cannotWrite5ByteInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeInt(0x123456789L, 5));
		}

		@Test
		void cannotWriteZeroSizeInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeInt(42, 0));
		}

		@Test
		void cannotWriteIntoExhaustedBuffer() {
			// Arrange:
			final Writer writer = new Writer(2);
			writer.writeInt(0xFFFF, 2);

			// Act + Assert:
			assertThrows(BufferOverflowException.class, () -> writer.writeInt(0xFF, 1));
		}

		@Test
		void canWrite0Value() {
			// Arrange:
			final Writer writer = new Writer(4);

			// Act:
			writer.writeInt(0, 4);

			// Assert:
			assertThat(writer.offset, equalTo(4));
			assertThat(writer.storage, equalTo(new byte[]{
					0, 0, 0, 0
			}));
		}
	}

	// endregion

	// region writeBigInt

	@Nested
	final class WriteBigInt {
		@Test
		void canWrite8ByteBigInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeBigInt(BigInteger.valueOf(0x123456789ABCDEFL), 8);

			// Assert:
			assertThat(writer.offset, equalTo(8));
			// Little-endian: 0xEF 0xCD 0xAB 0x89 0x67 0x45 0x23 0x01
			assertThat(writer.storage[0], equalTo((byte) 0xEF));
			assertThat(writer.storage[1], equalTo((byte) 0xCD));
			assertThat(writer.storage[2], equalTo((byte) 0xAB));
			assertThat(writer.storage[3], equalTo((byte) 0x89));
			assertThat(writer.storage[4], equalTo((byte) 0x67));
			assertThat(writer.storage[5], equalTo((byte) 0x45));
			assertThat(writer.storage[6], equalTo((byte) 0x23));
			assertThat(writer.storage[7], equalTo((byte) 0x01));
		}

		@Test
		void canWriteZeroBigInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeBigInt(BigInteger.ZERO, 8);

			// Assert:
			assertThat(writer.offset, equalTo(8));
			assertThat(writer.storage[0], equalTo((byte) 0));
			assertThat(writer.storage[1], equalTo((byte) 0));
			assertThat(writer.storage[2], equalTo((byte) 0));
			assertThat(writer.storage[3], equalTo((byte) 0));
			assertThat(writer.storage[4], equalTo((byte) 0));
			assertThat(writer.storage[5], equalTo((byte) 0));
			assertThat(writer.storage[6], equalTo((byte) 0));
			assertThat(writer.storage[7], equalTo((byte) 0));
		}

		@Test
		void canWriteLargeBigInt() {
			// Arrange:
			final Writer writer = new Writer(20);

			// Act:
			writer.writeBigInt(BigInteger.valueOf(Long.MAX_VALUE), 8);

			// Assert:
			assertThat(writer.offset, equalTo(8));
			// Long.MAX_VALUE = 0x7FFFFFFFFFFFFFFF
			assertThat(writer.storage[0], equalTo((byte) 0xFF));
			assertThat(writer.storage[1], equalTo((byte) 0xFF));
			assertThat(writer.storage[2], equalTo((byte) 0xFF));
			assertThat(writer.storage[3], equalTo((byte) 0xFF));
			assertThat(writer.storage[4], equalTo((byte) 0xFF));
			assertThat(writer.storage[5], equalTo((byte) 0xFF));
			assertThat(writer.storage[6], equalTo((byte) 0xFF));
			assertThat(writer.storage[7], equalTo((byte) 0x7F));
		}

		@Test
		void canWriteNegativeBigInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeBigInt(BigInteger.valueOf(-1), 8);

			// Assert:
			assertThat(writer.offset, equalTo(8));
			// -1 in two's complement: all bytes are 0xFF
			for (int i = 0; i < 8; ++i)
				assertThat(writer.storage[i], equalTo((byte) 0xFF));
		}

		@Test
		void canWriteMultipleBigInts() {
			// Arrange:
			final Writer writer = new Writer(20);

			// Act:
			writer.writeBigInt(BigInteger.ZERO, 8);
			writer.writeBigInt(BigInteger.ONE, 8);

			// Assert:
			assertThat(writer.offset, equalTo(16));
		}

		@Test
		void cannotWrite7ByteBigInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeBigInt(BigInteger.ONE, 7));
		}

		@Test
		void cannotWrite16ByteBigInt() {
			// Arrange:
			final Writer writer = new Writer(20);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeBigInt(BigInteger.ONE, 16));
		}

		@Test
		void cannotWriteZeroSizeBigInt() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeBigInt(BigInteger.ONE, 0));
		}

		@Test
		void cannotWriteIntoExhaustedBuffer() {
			// Arrange:
			final Writer writer = new Writer(8);
			writer.writeBigInt(BigInteger.ZERO, 8);

			// Act + Assert:
			assertThrows(BufferOverflowException.class, () -> writer.writeBigInt(BigInteger.ONE, 8));
		}

		@Test
		void cannotWriteBigIntIntoPartiallyExhaustedBuffer() {
			// Arrange:
			final Writer writer = new Writer(10);
			writer.write(new byte[]{
					1, 2, 3
			});

			// Act + Assert:
			assertThrows(BufferOverflowException.class, () -> writer.writeBigInt(BigInteger.ONE, 8));
		}
	}

	// endregion
}
