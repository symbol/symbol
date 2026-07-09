package org.symbol.sdk;

/**
 * Signals a failure inside the SDK's cryptographic primitives, typically wrapping a JDK {@link java.security.GeneralSecurityException}.
 * Extends {@link IllegalStateException} so it remains unchecked for existing callers.
 */
public final class CryptoException extends IllegalStateException {
	private static final long serialVersionUID = 1L;

	/**
	 * Creates a crypto exception.
	 *
	 * @param message Failure reason.
	 */
	public CryptoException(final String message) {
		super(message);
	}

	/**
	 * Creates a crypto exception wrapping an underlying cause.
	 *
	 * @param message Failure reason.
	 * @param cause Underlying cause.
	 */
	public CryptoException(final String message, final Throwable cause) {
		super(message, cause);
	}

	/**
	 * Creates a crypto exception wrapping an underlying cause (no explicit message).
	 *
	 * @param cause Underlying cause.
	 */
	public CryptoException(final Throwable cause) {
		super(cause);
	}
}
