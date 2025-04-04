package org.symbol.sdk.utils;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.*;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.nio.BufferOverflowException;
import org.junit.Test;

public class BufferWriterTest {

	@Test
	public void canConstructWithEmptyBuffer() {
		// Arrange:
		final Writer writer = new BufferWriter(0);

		// Act + Assert:
		final Exception exception = assertThrows(Exception.class, () -> writer.write(new byte[]{
				42
		}));
	}

	@Test
	public void canConstructWithNonEmptyBuffer() {
		// Arrange:
		final Writer writer = new BufferWriter(10);

		// Act:
		writer.write(new byte[]{
				42, 3, 14, 15
		});

		// Assert:
		assertThat(writer.getStorage(), equalTo(new byte[]{
				42, 3, 14, 15, 0, 0, 0, 0, 0, 0
		}));
	}

	@Test
	public void canMultipleWritesAreSaved() {
		// Arrange:
		final Writer writer = new BufferWriter(10);

		// Act:
		writer.write(new byte[]{
				42, 3, 14
		});
		writer.write(new byte[]{
				15, 92, 65, 35
		});

		// Assert:
		assertThat(writer.getStorage(), equalTo(new byte[]{
				42, 3, 14, 15, 92, 65, 35, 0, 0, 0
		}));
	}

	@Test
	public void canWriteUntilTheEndOfTheBuffer() {
		// Arrange:
		final Writer writer = new BufferWriter(10);

		// Act:
		writer.write(new byte[]{
				42, 3, 14, 15, 92, 65, 35
		});
		writer.write(new byte[]{
				89, 79, 32
		});

		// Assert:
		assertThat(writer.getStorage(), equalTo(new byte[]{
				42, 3, 14, 15, 92, 65, 35, 89, 79, 32
		}));
	}

	@Test
	public void cannotWritePastTheEndBuffer() {
		// Arrange:
		final Writer writer = new BufferWriter(10);
		final int initialWriteSize = (int) (Math.random() * 3) + 7; // number between 7 - 10
		writer.write(new byte[initialWriteSize]);

		// Act + Assert:
		assertThrows(BufferOverflowException.class, () -> writer.write(new byte[]{
				89, 79, 32, 38
		}));
	}
}
