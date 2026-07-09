package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.nullValue;
import static org.hamcrest.Matchers.sameInstance;

import java.io.IOException;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

final class CryptoExceptionTest {
	// region Inheritance

	@Nested
	final class Inheritance {
		@Test
		void extendsIllegalStateException() {
			// Act:
			final CryptoException actual = new CryptoException("test");

			// Assert:
			assertThat(actual, instanceOf(IllegalStateException.class));
		}

		@Test
		void isUnchecked() {
			// Arrange:
			final CryptoException ce = new CryptoException("test");

			// Act + Assert:
			// If it were checked, this would not compile
			assertThat(ce instanceof RuntimeException, equalTo(true));
		}
	}

	// endregion

	// region Message-Only Constructor

	@Nested
	final class MessageOnlyConstructor {
		@Test
		void canCreateWithMessage() {
			// Arrange:
			final String message = "cryptographic failure";

			// Act:
			final CryptoException ce = new CryptoException(message);

			// Assert:
			assertThat(ce.getMessage(), equalTo(message));
			assertThat(ce.getCause(), nullValue());
		}

		@Test
		void emptyMessageIsAccepted() {
			// Act:
			final CryptoException ce = new CryptoException("");

			// Assert:
			assertThat(ce.getMessage(), equalTo(""));
		}

		@Test
		void nullMessageIsAccepted() {
			// Act:
			final CryptoException ce = new CryptoException((String) null);

			// Assert:
			assertThat(ce.getMessage(), nullValue());
		}
	}

	// endregion

	// region Message and Cause Constructor

	@Nested
	final class MessageAndCauseConstructor {
		@Test
		void canCreateWithMessageAndCause() {
			// Arrange:
			final String message = "cryptographic failure";
			final IOException cause = new IOException("underlying I/O error");

			// Act:
			final CryptoException ce = new CryptoException(message, cause);

			// Assert:
			assertThat(ce.getMessage(), equalTo(message));
			assertThat(ce.getCause(), sameInstance(cause));
		}

		@Test
		void nullMessageWithCauseIsAccepted() {
			// Arrange:
			final IOException cause = new IOException("I/O error");

			// Act:
			final CryptoException ce = new CryptoException((String) null, cause);

			// Assert:
			assertThat(ce.getMessage(), nullValue());
			assertThat(ce.getCause(), sameInstance(cause));
		}

		@Test
		void nullCauseWithMessageIsAccepted() {
			// Arrange:
			final String message = "verification failed";

			// Act:
			final CryptoException ce = new CryptoException(message, null);

			// Assert:
			assertThat(ce.getMessage(), equalTo(message));
			assertThat(ce.getCause(), nullValue());
		}

		@Test
		void bothMessageAndCauseCanBeNull() {
			// Act:
			final CryptoException ce = new CryptoException((String) null, null);

			// Assert:
			assertThat(ce.getMessage(), nullValue());
			assertThat(ce.getCause(), nullValue());
		}
	}

	// endregion

	// region Cause-Only Constructor

	@Nested
	final class CauseOnlyConstructor {
		@Test
		void messageIsGeneratedFromCause() {
			// Arrange:
			final String message = "invalid input";
			final IllegalArgumentException cause = new IllegalArgumentException(message);

			// Act:
			final CryptoException ce = new CryptoException(cause);

			// Assert:
			// Message should be non-null (generated from cause)
			assertThat(ce.getMessage(), equalTo("java.lang.IllegalArgumentException: " + message));
			assertThat(ce.getCause(), sameInstance(cause));
		}

		@Test
		void nullCauseIsAccepted() {
			// Act:
			final CryptoException ce = new CryptoException((Throwable) null);

			// Assert: Should not throw
			assertThat(ce.getMessage(), equalTo(null));
			assertThat(ce.getCause(), nullValue());
		}
	}

	// endregion
}
