package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.math.BigInteger;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class BufferViewTest {
	private static BufferView newSubView() {
		final byte[] backing = {
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
		};
		// sub-view from offset 2 of length 5, materialized by copying
		final byte[] sub = new byte[5];
		System.arraycopy(backing, 2, sub, 0, 5);
		return new BufferView(sub);
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
		assertThat(view.buffer(), equalTo(bytes));
	}

	// region Accessors

	@Nested
	final class Accessors {
		@Test
		void backingReturnsUnderlyingArray() {
			// Arrange:
			final byte[] bytes = {
					0, 1, 2, 3, 4, 5, 6
			};
			final BufferView view = new BufferView(bytes);

			// Act + Assert:
			assertThat(view.backing() == bytes, equalTo(true));
		}

		@Test
		void offsetReturnsZeroForNewView() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0, 1, 2, 3
			});

			// Act + Assert:
			assertThat(view.offset(), equalTo(0));
		}

		@Test
		void offsetReturnsCorrectValueAfterShift() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(2);

			// Assert:
			assertThat(view.offset(), equalTo(2));
		}

		@Test
		void offsetReturnsCorrectValueForWindowedView() {
			// Arrange:
			final byte[] backing = new byte[]{
					0, 1, 2, 3, 4, 5, 6, 7
			};
			final BufferView view = new BufferView(backing, 3, 4);

			// Act + Assert:
			assertThat(view.offset(), equalTo(3));
		}

		@Test
		void lengthReturnsFullLengthForNewView() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0, 1, 2, 3
			});

			// Act + Assert:
			assertThat(view.length(), equalTo(4));
		}

		@Test
		void lengthDecreasesAfterShift() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(2);

			// Assert:
			assertThat(view.length(), equalTo(3));
		}

		@Test
		void lengthDecreasesAfterShrink() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shrink(3);

			// Assert:
			assertThat(view.length(), equalTo(3));
		}

		@Test
		void lengthReturnsCorrectValueForWindowedView() {
			// Arrange:
			final byte[] backing = new byte[]{
					0, 1, 2, 3, 4, 5, 6, 7
			};
			final BufferView view = new BufferView(backing, 2, 4);

			// Act + Assert:
			assertThat(view.length(), equalTo(4));
		}
	}

	// endregion

	// region ShiftRight

	@Nested
	final class ShiftRight {
		@Test
		void canShiftBy0() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(0);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{
					2, 3, 4, 5, 6
			}));
		}

		@Test
		void canShiftBy1() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(1);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{
					3, 4, 5, 6
			}));
		}

		@Test
		void canShiftByMoreThan1() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(3);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{
					5, 6
			}));
		}

		@Test
		void canShiftByWholeSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(5);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[0]));
		}

		@Test
		void canDoMultipleShifts() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shiftRight(2);
			view.shiftRight(2);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{
					6
			}));
		}

		@Test
		void cannotShiftPastBuffer() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.shiftRight(6));
		}

		@Test
		void cannotShiftNegative() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.shiftRight(-1));
		}
	}

	// endregion

	// region Window

	@Nested
	final class Window {
		@Test
		void createsSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThat(view.window(3), equalTo(new byte[]{
					2, 3, 4
			}));
		}

		@Test
		void canCreateNonShrinkingSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThat(view.window(5), equalTo(new byte[]{
					2, 3, 4, 5, 6
			}));
		}

		@Test
		void canCreateZeroSizeWindow() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThat(view.window(0), equalTo(new byte[]{}));
		}

		@Test
		void cannotCreateGrowingSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.window(6));
		}

		@Test
		void cannotCreateNegativeSizeWindow() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.window(-1));
		}
	}

	// endregion

	// region Shrink

	@Nested
	final class Shrink {
		@Test
		void createsSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shrink(3);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{
					2, 3, 4
			}));
		}

		@Test
		void canCreateNonShrinkingSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shrink(5);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{
					2, 3, 4, 5, 6
			}));
		}

		@Test
		void canShrinkToZero() {
			// Arrange:
			final BufferView view = newSubView();

			// Act:
			view.shrink(0);

			// Assert:
			assertThat(view.buffer(), equalTo(new byte[]{}));
		}

		@Test
		void cannotCreateGrowingSubview() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.shrink(6));
		}

		@Test
		void cannotShrinkNegative() {
			// Arrange:
			final BufferView view = newSubView();

			// Act + Assert:
			assertThrows(IndexOutOfBoundsException.class, () -> view.shrink(-1));
		}
	}

	// endregion

	// region PeekInt

	@Nested
	final class PeekInt {
		@Test
		void canPeek1ByteUnsigned() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, 0x01, 0x02
			});

			// Act:
			final long value = view.peekInt(1, false);

			// Assert:
			assertThat(value, equalTo(255L));
		}

		@Test
		void canPeek1ByteSigned() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, 0x01, 0x02
			});

			// Act:
			final long value = view.peekInt(1, true);

			// Assert:
			assertThat(value, equalTo(-1L));
		}

		@Test
		void canPeek2ByteUnsignedLittleEndian() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x34, 0x12, 0x00
			});

			// Act:
			final long value = view.peekInt(2, false);

			// Assert:
			assertThat(value, equalTo(0x1234L));
		}

		@Test
		void canPeek2ByteSignedLittleEndian() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, (byte) 0xFF, 0x00
			});

			// Act:
			final long value = view.peekInt(2, true);

			// Assert:
			assertThat(value, equalTo(-1L));
		}

		@Test
		void canPeek4ByteUnsignedLittleEndian() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x78, 0x56, 0x34, 0x12, 0x00
			});

			// Act:
			final long value = view.peekInt(4, false);

			// Assert:
			assertThat(value, equalTo(0x12345678L));
		}

		@Test
		void canPeek4ByteSignedLittleEndian() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, 0x00
			});

			// Act:
			final long value = view.peekInt(4, true);

			// Assert:
			assertThat(value, equalTo(-1L));
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
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03
			});

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekInt(3, false));
		}

		@Test
		void cannotPeek5ByteInt() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05
			});

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekInt(5, false));
		}

		@Test
		void cannotPeekZeroSizeInt() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02
			});

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekInt(0, false));
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

	// region PeekBigInt

	@Nested
	final class PeekBigInt {
		@Test
		void canPeek8ByteUnsignedLittleEndian() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xEF, (byte) 0xCD, (byte) 0xAB, (byte) 0x89, (byte) 0x67, (byte) 0x45, (byte) 0x23, (byte) 0x01, 0x00
			});

			// Act:
			final BigInteger value = view.peekBigInt(8, false);

			// Assert:
			assertThat(value, equalTo(BigInteger.valueOf(0x123456789ABCDEFL)));
		}

		@Test
		void canPeek8ByteSignedPositive() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00
			});

			// Act:
			final BigInteger value = view.peekBigInt(8, true);

			// Assert:
			assertThat(value.longValue(), equalTo(0x0807060504030201L));
		}

		@Test
		void canPeek8ByteSignedNegative() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, (byte) 0xFF, 0x00
			});

			// Act:
			final BigInteger value = view.peekBigInt(8, true);

			// Assert:
			assertThat(value, equalTo(BigInteger.valueOf(-1)));
		}

		@Test
		void canPeekZeroBigInt() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			});

			// Act:
			final BigInteger value = view.peekBigInt(8, false);

			// Assert:
			assertThat(value, equalTo(BigInteger.ZERO));
		}

		@Test
		void peekBigIntDoesNotAdvance() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x00
			});

			// Act:
			final BigInteger value1 = view.peekBigInt(8, false);
			final BigInteger value2 = view.peekBigInt(8, false);

			// Assert:
			assertThat(value1, equalTo(value2));
			assertThat(view.length(), equalTo(9));
		}

		@Test
		void peekBigIntWorksAfterShift() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					(byte) 0xFF, (byte) 0xEF, (byte) 0xCD, (byte) 0xAB, (byte) 0x89, (byte) 0x67, (byte) 0x45, (byte) 0x23, (byte) 0x01
			});
			view.shiftRight(1);

			// Act:
			final BigInteger value = view.peekBigInt(8, false);

			// Assert:
			assertThat(value, equalTo(BigInteger.valueOf(0x123456789ABCDEFL)));
		}

		@Test
		void cannotPeek7ByteBigInt() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
			});

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekBigInt(7, false));
		}

		@Test
		void cannotPeek16ByteBigInt() {
			// Arrange:
			final BufferView view = new BufferView(new byte[16]);

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekBigInt(16, false));
		}

		@Test
		void cannotPeekZeroSizeBigInt() {
			// Arrange:
			final BufferView view = new BufferView(new byte[]{
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
			});

			// Act + Assert:
			assertThrows(IllegalArgumentException.class, () -> view.peekBigInt(0, false));
		}

		@Test
		void unsignedNegativeLongBecomesPositiveBigInteger() {
			// Arrange: Long.MAX_VALUE + 1 as unsigned (0x8000000000000000)
			final BufferView view = new BufferView(new byte[]{
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, (byte) 0x80, 0x00
			});

			// Act:
			final BigInteger value = view.peekBigInt(8, false);

			// Assert: Should be 2^63
			assertThat(value.compareTo(BigInteger.ZERO), equalTo(1));
			assertThat(value.compareTo(BigInteger.valueOf(Long.MAX_VALUE)), equalTo(1));
		}
	}

	// endregion

	@Test
	void negativeSizesAreRejected() {
		// Arrange:
		final BufferView view = new BufferView(new byte[]{
				1, 2, 3, 4
		});

		// Act + Assert:
		org.junit.jupiter.api.Assertions.assertThrows(IndexOutOfBoundsException.class, () -> view.shiftRight(-1));
		org.junit.jupiter.api.Assertions.assertThrows(IndexOutOfBoundsException.class, () -> view.window(-1));
		org.junit.jupiter.api.Assertions.assertThrows(IndexOutOfBoundsException.class, () -> view.shrink(-1));
	}

}
