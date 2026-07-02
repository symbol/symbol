package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class BufferViewTest {
	// a fresh view over the five bytes {2, 3, 4, 5, 6} (offset 0, length 5)
	private static BufferView newView() {
		return new BufferView(new byte[]{
				2, 3, 4, 5, 6
		});
	}

	@Test
	void canCreateAroundWholeBuffer() {
		// Arrange:
		final byte[] bytes = {
				0, 1, 2, 3, 4, 5, 6
		};

		// Act:
		final BufferView view = new BufferView(bytes);

		// Assert:
		assertThat(view.peekBytes(view.length()), equalTo(bytes));
		assertThat(view.length(), equalTo(7));
	}

	// region ShiftRight

	@Nested
	final class ShiftRight {
		private static void assertShiftRightYields(final int shift, final byte[] expected) {
			// Arrange:
			final BufferView view = newView();
			final int originalLength = view.length();

			// Act:
			view.shiftRight(shift);

			// Assert:
			assertThat(view.peekBytes(view.length()), equalTo(expected));
			assertThat(originalLength - shift, equalTo(view.length()));
		}

		private static void assertShiftRightThrows(final int shift) {
			// Arrange:
			final BufferView view = newView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.shiftRight(shift));
		}

		@Test
		void canShiftBy0() {
			assertShiftRightYields(0, new byte[]{
					2, 3, 4, 5, 6
			});
		}

		@Test
		void canShiftBy1() {
			assertShiftRightYields(1, new byte[]{
					3, 4, 5, 6
			});
		}

		@Test
		void canShiftByMoreThan1() {
			assertShiftRightYields(3, new byte[]{
					5, 6
			});
		}

		@Test
		void canShiftByWholeSubview() {
			assertShiftRightYields(5, new byte[0]);
		}

		@Test
		void canDoMultipleShifts() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			view.shiftRight(2);
			view.shiftRight(2);

			// Assert:
			assertThat(view.peekBytes(view.length()), equalTo(new byte[]{
					6
			}));
		}

		@Test
		void cannotShiftPastBuffer() {
			assertShiftRightThrows(6);
		}

		@Test
		void cannotShiftNegative() {
			assertShiftRightThrows(-1);
		}
	}

	// endregion

	// region Window

	@Nested
	final class Window {
		private static void assertWindowThrows(final int size) {
			// Arrange:
			final BufferView view = newView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.window(size));
		}

		@Test
		void createsSubview() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			final BufferView window = view.window(3);

			// Assert: zero-copy and rebased — the window exposes the first 3 bytes starting at its own first byte.
			assertThat(window.length(), equalTo(3));
			assertThat(window.peekBytes(window.length()), equalTo(new byte[]{
					2, 3, 4
			}));
		}

		@Test
		void sharesBackingWithoutCopying() {
			// Arrange:
			final byte[] backing = {
					2, 3, 4, 5, 6
			};
			final BufferView view = new BufferView(backing);
			final BufferView window = view.window(3);

			// Act: mutate the source array after the window exists.
			backing[0] = 99;

			// Assert: the window observes the mutation, proving it reads the same storage rather than a copy.
			assertThat(window.peekBytes(1), equalTo(new byte[]{
					99
			}));
		}

		@Test
		void canCreateNonShrinkingSubview() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			final BufferView window = view.window(5);

			// Assert:
			assertThat(window.peekBytes(5), equalTo(new byte[]{
					2, 3, 4, 5, 6
			}));
		}

		@Test
		void canCreateShrinkingSubview() {
			// Arrange:
			final BufferView view = newView();
			view.shiftRight(3);

			// Act:
			final BufferView window = view.window(2);

			// Assert: the window is rebased — reads start at its first byte regardless of where it sits in the backing.
			assertThat(window.length(), equalTo(2));
			assertThat(window.peekBytes(2), equalTo(new byte[]{
					5, 6
			}));
		}

		@Test
		void canCreateZeroSizeWindow() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			final BufferView window = view.window(0);

			// Assert:
			assertThat(window.peekBytes(0), equalTo(new byte[]{}));
		}

		@Test
		void cannotCreateGrowingSubview() {
			assertWindowThrows(6);
		}

		@Test
		void cannotCreateNegativeSizeWindow() {
			assertWindowThrows(-1);
		}
	}

	// endregion

	// region Snapshot

	@Nested
	final class Snapshot {
		@Test
		void copiesCurrentWindow() {
			// Arrange:
			final BufferView view = newView();
			view.shiftRight(1);

			// Act:
			final BufferView snapshot = view.snapshot();

			// Assert: rebased copy of the source's current window — same length and bytes.
			assertThat(snapshot.length(), equalTo(view.length()));
			assertThat(snapshot.peekBytes(snapshot.length()), equalTo(new byte[]{
					3, 4, 5, 6
			}));
			assertThat(view.peekBytes(snapshot.length()), equalTo(new byte[]{
					3, 4, 5, 6
			}));
		}

		@Test
		void sharesBackingWithoutCopying() {
			// Arrange:
			final byte[] backing = {
					2, 3, 4, 5, 6
			};
			final BufferView view = new BufferView(backing);
			final BufferView snapshot = view.snapshot();

			// Act: mutate the source array after the window exists.
			backing[0] = 99;

			// Assert: the window observes the mutation, proving it reads the same storage rather than a copy.
			assertThat(snapshot.peekBytes(1), equalTo(new byte[]{
					99
			}));
		}

		@Test
		void isIndependentOfTheSource() {
			// Arrange:
			final BufferView view = newView();
			final BufferView snapshot = view.snapshot();

			// Act: advancing the snapshot must not move the source.
			snapshot.shiftRight(2);

			// Assert: the source still sees all five bytes; only the snapshot advanced.
			assertThat(view.length(), equalTo(5));
			assertThat(view.peekBytes(view.length()), equalTo(new byte[]{
					2, 3, 4, 5, 6
			}));
			assertThat(snapshot.peekBytes(snapshot.length()), equalTo(new byte[]{
					4, 5, 6
			}));
		}
	}

	// endregion

	// region PeekBytes

	@Nested
	final class PeekBytes {
		private static void assertPeekBytesThrows(final int size) {
			// Arrange:
			final BufferView view = newView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.peekBytes(size));
		}

		@Test
		void copiesLeadingBytesWithoutAdvancing() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			final byte[] bytes = view.peekBytes(3);

			// Assert: the first 3 bytes are copied and the position is unchanged
			assertThat(bytes, equalTo(new byte[]{
					2, 3, 4
			}));
			assertThat(view.length(), equalTo(5));
		}

		@Test
		void canPeekZeroBytes() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			final byte[] actual = view.peekBytes(0);

			// Assert:
			assertThat(actual, equalTo(new byte[]{}));
		}

		@Test
		void canPeekWholeWindow() {
			// Arrange:
			final BufferView view = newView();

			// Act:
			final byte[] actual = view.peekBytes(5);

			// Assert:
			assertThat(actual, equalTo(new byte[]{
					2, 3, 4, 5, 6
			}));
		}

		@Test
		void cannotPeekPastRemaining() {
			assertPeekBytesThrows(6);
		}

		@Test
		void cannotPeekNegativeSize() {
			assertPeekBytesThrows(-1);
		}
	}

	// endregion

	// region Shrink

	@Nested
	final class Shrink {
		private static void assertShrinkYields(final int size, final byte[] expected) {
			// Arrange:
			final BufferView view = newView();

			// Act:
			view.shrink(size);

			// Assert:
			assertThat(view.peekBytes(view.length()), equalTo(expected));
			assertThat(view.length(), equalTo(size));
		}

		private static void assertShrinkThrows(final int size) {
			// Arrange:
			final BufferView view = newView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.shrink(size));
		}

		@Test
		void createsSubview() {
			assertShrinkYields(3, new byte[]{
					2, 3, 4
			});
		}

		@Test
		void canCreateNonShrinkingSubview() {
			assertShrinkYields(5, new byte[]{
					2, 3, 4, 5, 6
			});
		}

		@Test
		void canShrinkToZero() {
			assertShrinkYields(0, new byte[]{});
		}

		@Test
		void canShrinkAfterShift() {
			// Arrange:
			final BufferView view = newView();
			view.shiftRight(3);

			// Act:
			view.shrink(2);

			// Assert: shrink rebases and bounds the window to the two bytes at the shifted position.
			assertThat(view.length(), equalTo(2));
			assertThat(view.peekBytes(view.length()), equalTo(new byte[]{
					5, 6
			}));
		}

		@Test
		void cannotCreateGrowingSubview() {
			assertShrinkThrows(6);
		}

		@Test
		void cannotShrinkNegative() {
			assertShrinkThrows(-1);
		}
	}

	// endregion

	// region PeekInt

	@Nested
	final class PeekInt {
		private void assertPeekInt(final byte[] bytes, final int size, final boolean signed, final long expected) {
			// Act:
			final long value = new BufferView(bytes).peekInt(size, signed);

			// Assert:
			assertThat(value, equalTo(expected));
		}

		private void assertCannotPeekByteInt(final int size) {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09
			});

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekInt(size, false));
		}

		@Test
		void canPeek1ByteUnsigned() {
			assertPeekInt(new byte[]{
					(byte) 0xFF, 0x01, 0x02
			}, 1, false, 255L);
		}

		@Test
		void canPeek1ByteSigned() {
			assertPeekInt(new byte[]{
					(byte) 0xFF, 0x01, 0x02
			}, 1, true, -1L);
		}

		@Test
		void canPeek2ByteUnsignedLittleEndian() {
			assertPeekInt(new byte[]{
					0x34, 0x12, 0x00
			}, 2, false, 0x1234L);
		}

		@Test
		void canPeek2ByteSignedLittleEndian() {
			assertPeekInt(new byte[]{
					(byte) 0xFF, (byte) 0xFF, 0x00
			}, 2, true, -1L);
		}

		@Test
		void canPeek4ByteUnsignedLittleEndian() {
			assertPeekInt(new byte[]{
					0x78, 0x56, 0x34, 0x12, 0x00
			}, 4, false, 0x12345678L);
		}

		@Test
		void canPeek4ByteSignedLittleEndian() {
			assertPeekInt(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, 0x00
			}, 4, true, -1L);
		}

		@Test
		void canPeek8ByteLittleEndianU64ReadsBackNegative() {
			// Arrange: a u64 >= 2^63 (most-significant byte 0xEF) laid out little-endian.
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x23, 0x45, 0x67, (byte) 0x89, (byte) 0xAB, (byte) 0xCD, (byte) 0xEF, 0x00
			});

			// Act:
			final long unsignedView = view.peekInt(8, false);
			final long signedView = view.peekInt(8, true);

			// Assert: the full 64-bit pattern is returned; isSigned is a no-op at size 8, and a u64 >= 2^63 reads back negative.
			assertThat(unsignedView, equalTo(0xEFCDAB8967452301L));
			assertThat(signedView, equalTo(unsignedView));
			assertThat(unsignedView < 0L, equalTo(true));
		}

		@Test
		void peekDoesNotAdvance() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x12, 0x34
			});

			// Act:
			final long value1 = view.peekInt(1, false);
			final long value2 = view.peekInt(1, false);

			// Assert:
			assertThat(value1, equalTo(value2));
			assertThat(view.length(), equalTo(2));
		}

		@Test
		void peekWorksAfterShift() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, 0x34, 0x12
			});
			view.shiftRight(1);

			// Act:
			final long value = view.peekInt(2, false);

			// Assert:
			assertThat(value, equalTo(0x1234L));
		}

		@Test
		void cannotPeek3ByteInt() {
			assertCannotPeekByteInt(3);
		}

		@Test
		void cannotPeek5ByteInt() {
			assertCannotPeekByteInt(5);
		}

		@Test
		void cannotPeek9ByteInt() {
			assertCannotPeekByteInt(9);
		}

		@Test
		void cannotPeekZeroSizeInt() {
			assertCannotPeekByteInt(0);
		}

		@Test
		void cannotPeekIntPastRemaining() {
			// Arrange: a valid width (8) larger than the available bytes.
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04
			});

			// Act + Assert: the window bounds check fires before the read (clear message, not a raw ByteBuffer error).
			assertThrows(IndexOutOfBoundsException.class, () -> view.peekInt(8, false));
		}

		@Test
		void peekAtDifferentPositions() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x11, 0x22, 0x33, 0x44
			});
			view.shiftRight(1);

			// Act:
			final long value1 = view.peekInt(1, false);
			view.shiftRight(1);
			final long value2 = view.peekInt(1, false);

			// Assert:
			assertThat(value1, equalTo(0x22L));
			assertThat(value2, equalTo(0x33L));
		}
	}

	// endregion
}
