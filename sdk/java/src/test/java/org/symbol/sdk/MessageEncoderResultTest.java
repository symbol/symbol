package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.sameInstance;

import org.junit.jupiter.api.Test;

/** Tests {@link MessageEncoderResult}. */
final class MessageEncoderResultTest {
	@Test
	void exposesDecodedFlagAndMessage() {
		// Arrange:
		final byte[] clear = {
				1, 2, 3
		};

		// Act:
		final MessageEncoderResult result = new MessageEncoderResult(true, clear);

		// Assert:
		assertThat(result.isDecoded(), is(true));
		assertThat(result.message(), sameInstance(clear));
	}

	@Test
	void carriesOriginalPayloadWhenNotDecoded() {
		// Arrange:
		final Object payload = new Object();

		// Act:
		final MessageEncoderResult result = new MessageEncoderResult(false, payload);

		// Assert:
		assertThat(result.isDecoded(), is(false));
		assertThat(result.message(), sameInstance(payload));
	}

	@Test
	void equalsAndHashCodeFollowComponents() {
		// Arrange:
		final Object msg = "payload";
		final MessageEncoderResult a = new MessageEncoderResult(true, msg);
		final MessageEncoderResult b = new MessageEncoderResult(true, msg);

		// Assert:
		assertThat(a, equalTo(b));
		assertThat(a.hashCode(), equalTo(b.hashCode()));
		assertThat(a.equals(new MessageEncoderResult(false, msg)), is(false));
	}

	@Test
	void equalsComparesByteArrayMessagesByContent() {
		// Arrange: a byte[] message (the Symbol decoded/echoed form) must compare by content, not array identity
		final MessageEncoderResult a = new MessageEncoderResult(true, new byte[]{
				1, 2, 3
		});
		final MessageEncoderResult b = new MessageEncoderResult(true, new byte[]{
				1, 2, 3
		});
		final MessageEncoderResult different = new MessageEncoderResult(true, new byte[]{
				1, 2, 4
		});

		// Assert:
		assertThat(a, equalTo(b));
		assertThat(a.hashCode(), equalTo(b.hashCode()));
		assertThat(a.equals(different), is(false));
	}
}
