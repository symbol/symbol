package org.symbol.sdk;

import java.util.Arrays;
import java.util.Objects;

/**
 * Result of a {@code tryDecode} operation on a {@code MessageEncoder}. When not decoded, {@code message} echoes back the original encoded
 * payload (raw {@code byte[]} for Symbol; original {@code Message} object for NEM).
 *
 * @param isDecoded {@code true} when the message was successfully decoded and decrypted.
 * @param message Cleartext bytes when decoded; original encoded payload otherwise.
 */
public record MessageEncoderResult(boolean isDecoded, Object message) {
	@Override
	public boolean equals(final Object other) {
		if (this == other)
			return true;

		if (!(other instanceof MessageEncoderResult otherResult) || isDecoded != otherResult.isDecoded)
			return false;

		// message is commonly a byte[] (Symbol cleartext/payload), whose default equals is identity-based; compare by content
		if (message instanceof byte[] bytes && otherResult.message instanceof byte[] otherBytes)
			return Arrays.equals(bytes, otherBytes);

		return Objects.equals(message, otherResult.message);
	}

	@Override
	public int hashCode() {
		return Objects.hash(isDecoded, message instanceof byte[] bytes ? Arrays.hashCode(bytes) : message);
	}
}
