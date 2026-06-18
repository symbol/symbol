package org.symbol.sdk;

/**
 * Signals that a descriptor could not be converted into an entity — a missing key, a wrong-typed value, or a failed typed look-up. Extends
 * {@link IllegalArgumentException} so existing {@code catch (IllegalArgumentException)} blocks keep working.
 */
public final class InvalidDescriptorException extends IllegalArgumentException {
	private static final long serialVersionUID = 1L;

	/**
	 * Creates an invalid-descriptor exception.
	 *
	 * @param message Failure reason.
	 */
	public InvalidDescriptorException(final String message) {
		super(message);
	}

	/**
	 * Creates an invalid-descriptor exception wrapping an underlying cause.
	 *
	 * @param message Failure reason.
	 * @param cause Underlying cause.
	 */
	public InvalidDescriptorException(final String message, final Throwable cause) {
		super(message, cause);
	}
}
