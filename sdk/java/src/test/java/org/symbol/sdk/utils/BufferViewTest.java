package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.function.Function;
import org.junit.Test;

public class BufferViewTest {
	// region constructor

	@Test
	public void canCreateBufferViewFromByteArray() {
		// Arrange:
		final byte[] byteArray = {
				0, 1, 2, 3, 4, 5, 6
		};

		// Act:
		final BufferView view = BufferView.wrap(byteArray);

		// Assert:
		assertThat(view.getBufferContent(), equalTo(new byte[]{
				0, 1, 2, 3, 4, 5, 6
		}));
	}

	@Test
	public void canCreateBufferViewFromEmptyByteArray() {
		// Arrange:
		final byte[] byteArray = {};

		// Act:
		final BufferView view = BufferView.wrap(byteArray);

		// Assert:
		assertThat(view.getBufferContent(), equalTo(new byte[]{}));
	}

	@Test
	public void canCreateBufferViewFromBuffer() {
		// Arrange:
		final byte[] byteArray = {
				0, 1, 2, 3, 4, 5, 6
		};
		final ByteBuffer buffer = ByteBuffer.wrap(byteArray);

		// Act:
		final BufferView view = BufferView.wrap(buffer);

		// Assert:
		assertThat(view.getBufferContent(), equalTo(new byte[]{
				0, 1, 2, 3, 4, 5, 6
		}));
	}

	// endregion

	// region shiftRight

	class TestContext {
		final byte[] byteArray = {
				0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
		};
		final ByteBuffer subView = ByteBuffer.wrap(byteArray, 2, 5);
		final BufferView view = BufferView.wrap(subView);
		final ByteBuffer slice = subView.slice();
		final byte[] sliceArray = slice.array();
		final ByteBuffer duplicate = subView.duplicate();
	}

	void assertShift(final int shiftCount, final byte[] expectedBuffer) {
		// Arrange:
		final TestContext context = new TestContext();

		// Act:
		context.view.shiftRight(shiftCount);
		final byte[] result = context.view.getBufferContent();

		// Assert:
		assertThat(result, equalTo(expectedBuffer));

		// - underlying buffer is the same
		assertThat(context.view.getBuffer().array(), equalTo(context.byteArray));
		byte[] newArray = Arrays.copyOfRange(context.byteArray, 2 + shiftCount, 2 + 5);
		assertThat(newArray, equalTo(result));
	};

	@Test
	public void canShiftZero() {
		assertShift(0, new byte[]{
				2, 3, 4, 5, 6
		});
	}

	@Test
	public void canShiftOne() {
		assertShift(1, new byte[]{
				3, 4, 5, 6
		});
	}

	@Test
	public void canShiftByMoreThanOne() {
		assertShift(3, new byte[]{
				5, 6
		});
	}

	@Test
	public void canShiftToEnd() {
		assertShift(5, new byte[0]);
	}

	@Test
	public void canDoMultipleShift() {
		// Arrange:
		final TestContext context = new TestContext();

		// Act:
		context.view.shiftRight(2);
		context.view.shiftRight(2);
		final byte[] result = context.view.getBufferContent();

		// Assert:
		assertThat(result, equalTo(new byte[]{
				6
		}));

		// - underlying buffer is the same
		assertThat(context.view.getBuffer().array(), equalTo(context.byteArray));
		byte[] newArray = Arrays.copyOfRange(context.byteArray, 2 + 4, 2 + 5);
		assertThat(newArray, equalTo(result));
	}

	@Test
	public void cannotShiftOutSideView() {
		// Arrange:
		final TestContext context = new TestContext();

		// Act + Assert:
		final IllegalArgumentException exception = assertThrows(IllegalArgumentException.class, () -> context.view.shiftRight(6));
		assertThat(exception.getMessage(), equalTo("size cannot be greater than the remaining bytes"));
	}

	@Test
	public void cannotShiftOutSideViewWithMultipleShifts() {
		// Arrange:
		final TestContext context = new TestContext();
		context.view.shiftRight(4);

		// Act + Assert:
		final IllegalArgumentException exception = assertThrows(IllegalArgumentException.class, () -> context.view.shiftRight(2));
		assertThat(exception.getMessage(), equalTo("size cannot be greater than the remaining bytes"));
	}

	// endregion

	// region window

	void assertModifyTest(final Function<BufferView, BufferView> mutator, final byte[] expected, final int newSize) {
		// Arrange:
		final TestContext context = new TestContext();

		// Act:
		final byte[] result = mutator.apply(context.view).getBufferContent();

		// Assert:
		assertThat(result, equalTo(expected));

		// - underlying buffer is the same
		byte[] newArray = Arrays.copyOfRange(context.byteArray, 2, 2 + newSize);
		assertThat(newArray, equalTo(result));
	};

	void assertWindowedBuffer(final int newSize, final byte[] expectedBuffer) {
		assertModifyTest(view -> view.window(newSize), expectedBuffer, newSize);
	};

	@Test
	public void canCreateSubview() {
		assertWindowedBuffer(3, new byte[]{
				2, 3, 4
		});
	}

	@Test
	public void canCreateNonShrinkingSubview() {
		assertWindowedBuffer(5, new byte[]{
				2, 3, 4, 5, 6
		});
	}

	@Test
	public void cannotCreateGrowingSubview() {
		// Arrange:
		final TestContext context = new TestContext();

		// Act + Assert:
		final IllegalArgumentException exception = assertThrows(IllegalArgumentException.class, () -> context.view.window(6));
		assertThat(exception.getMessage(), equalTo("size cannot be greater than the remaining bytes"));
	}

	@Test
	public void cannotCreateNegativeSubview() {
		// Arrange:
		final TestContext context = new TestContext();

		// Act + Assert:
		final IllegalArgumentException exception = assertThrows(IllegalArgumentException.class, () -> context.view.window(-1));
		assertThat(exception.getMessage(), equalTo("size cannot be negative"));
	}

	// endregion

	// region shrink

	void assertShrink(final int newSize, final byte[] expectedBuffer) {
		assertModifyTest(view -> {
			view.shrink(newSize);
			return view;
		}, expectedBuffer, newSize);
	}

	@Test
	public void canShrinkSubview() {
		assertShrink(3, new byte[]{
				2, 3, 4
		});
	}

	@Test
	public void canShrinkNonShrinkingSubview() {
		assertShrink(5, new byte[]{
				2, 3, 4, 5, 6
		});
	}

	@Test
	public void cannotShrinkSubview() {
		// Arrange:
		final TestContext context = new TestContext();

		// Act + Assert:
		final IllegalArgumentException exception = assertThrows(IllegalArgumentException.class, () -> context.view.window(6));
		assertThat(exception.getMessage(), equalTo("size cannot be greater than the remaining bytes"));
	}

	@Test
	public void cannotShrinkNegativeSubview() {
		// Arrange:
		final TestContext context = new TestContext();

		// Act + Assert:
		final IllegalArgumentException exception = assertThrows(IllegalArgumentException.class, () -> context.view.window(-1));
		assertThat(exception.getMessage(), equalTo("size cannot be negative"));
	}
}
