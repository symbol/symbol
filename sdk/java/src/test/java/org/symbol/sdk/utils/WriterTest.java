package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

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
			assertThat(writer.offset(), equalTo(4));
			assertThat(writer.storage(), equalTo(new byte[]{
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
			assertThat(writer.offset(), equalTo(7));
			assertThat(writer.storage(), equalTo(new byte[]{
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
			assertThat(writer.offset(), equalTo(10));
			assertThat(writer.storage(), equalTo(new byte[]{
					42, 3, 14, 15, 92, 65, 35, 89, 79, 32
			}));
		}

		private void assertCannotWritePastTheEnd(final int initialWriteSize) {
			// Arrange:
			final Writer writer = new Writer(10);
			writer.write(new byte[initialWriteSize]);

			// Sanity:
			assertThat(writer.offset(), equalTo(initialWriteSize));

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> writer.write(new byte[]{
					89, 79, 32, 38
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
			assertThat(writer.offset(), equalTo(0));
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
			assertThat(writer.offset(), equalTo(1));
			assertThat(writer.storage()[0], equalTo((byte) 42));
		}

		@Test
		void canWrite2ByteIntLittleEndian() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(0x1234, 2);

			// Assert:
			assertThat(writer.offset(), equalTo(2));
			// Little-endian: 0x34 0x12
			assertThat(writer.storage()[0], equalTo((byte) 0x34));
			assertThat(writer.storage()[1], equalTo((byte) 0x12));
		}

		@Test
		void canWrite4ByteIntLittleEndian() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(0x12345678L, 4);

			// Assert:
			assertThat(writer.offset(), equalTo(4));
			// Little-endian: 0x78 0x56 0x34 0x12
			assertThat(writer.storage()[0], equalTo((byte) 0x78));
			assertThat(writer.storage()[1], equalTo((byte) 0x56));
			assertThat(writer.storage()[2], equalTo((byte) 0x34));
			assertThat(writer.storage()[3], equalTo((byte) 0x12));
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
			assertThat(writer.offset(), equalTo(7));
			assertThat(writer.storage()[0], equalTo((byte) 0xFF));
			assertThat(writer.storage()[1], equalTo((byte) 0x34));
			assertThat(writer.storage()[2], equalTo((byte) 0x12));
			assertThat(writer.storage()[3], equalTo((byte) 0x78));
			assertThat(writer.storage()[4], equalTo((byte) 0x56));
			assertThat(writer.storage()[5], equalTo((byte) 0x34));
			assertThat(writer.storage()[6], equalTo((byte) 0x12));
		}

		private void assertCanWriteByteNegativeValue(final int size) {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act:
			writer.writeInt(-1, size);

			// Assert:
			assertThat(writer.offset(), equalTo(size));
			for (int i = 0; i < size; i++)
				assertThat(writer.storage()[i], equalTo((byte) 0xFF));
		}

		@Test
		void canWrite1ByteNegativeValue() {
			assertCanWriteByteNegativeValue(1);
		}

		@Test
		void canWrite2ByteNegativeValue() {
			assertCanWriteByteNegativeValue(2);
		}

		@Test
		void canWrite4ByteNegativeValue() {
			assertCanWriteByteNegativeValue(4);
		}

		@Test
		void canWrite8ByteNegativeValue() {
			assertCanWriteByteNegativeValue(8);
		}

		@Test
		void cannotWrite3ByteInt() {
			assertCannotWriteIntOfSize(0x123456, 3);
		}

		@Test
		void cannotWrite5ByteInt() {
			assertCannotWriteIntOfSize(0x123456789L, 5);
		}

		@Test
		void cannotWriteZeroSizeInt() {
			assertCannotWriteIntOfSize(42, 0);
		}

		private static void assertCannotWriteIntOfSize(final long value, final int size) {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> writer.writeInt(value, size));
		}

		@Test
		void cannotWriteIntoExhaustedBuffer() {
			// Arrange:
			final Writer writer = new Writer(2);
			writer.writeInt(0xFFFF, 2);

			// Act + Assert: the same clean bounds error write() raises, not a bare BufferOverflowException
			assertThrows(IndexOutOfBoundsException.class, () -> writer.writeInt(0xFF, 1));
		}

		@Test
		void cannotWriteValueThatDoesNotFitInSize() {
			// Arrange:
			final Writer writer = new Writer(10);

			// Act + Assert: 0x1FF round-trips as neither an unsigned nor a signed 1-byte value, so it is rejected
			// rather than silently truncated to 0xFF (0xFF and -1 still fit — see canWriteMultipleInts / negative cases)
			assertThrows(IllegalArgumentException.class, () -> writer.writeInt(0x1FF, 1));
			assertThrows(IllegalArgumentException.class, () -> writer.writeInt(0x10000, 2));
		}

		@Test
		void canWrite0Value() {
			// Arrange:
			final Writer writer = new Writer(4);

			// Act:
			writer.writeInt(0, 4);

			// Assert:
			assertThat(writer.offset(), equalTo(4));
			assertThat(writer.storage(), equalTo(new byte[]{
					0, 0, 0, 0
			}));
		}
	}

	// endregion
}
