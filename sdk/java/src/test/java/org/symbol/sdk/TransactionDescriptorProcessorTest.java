package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

/**
 * Tests for {@link TransactionDescriptorProcessor}. The nested classes mirror the describe blocks in the JS reference
 * ({@code TransactionDescriptorProcessor_spec.js}): {@code lookupValue}, {@code copyTo} and {@code setTypeHints}.
 */
final class TransactionDescriptorProcessorTest {
	// region test utils

	/**
	 * Test stand-in for a generated catbuffer struct: emits the {@code setField}/{@code getField} switch shape the generator produces — the
	 * only path {@link TransactionDescriptorProcessor#copyTo} supports.
	 */
	private static class Bag extends CatbufferType {
		Object type;
		Object timestamp;
		Object signer;
		Object recipient;
		Object message;

		@Override
		public int size() {
			return 0;
		}

		@Override
		protected void serializeInto(final org.symbol.sdk.utils.Writer buffer) {
		}

		@Override
		public Object toJson() {
			return java.util.Map.of();
		}

		@Override
		public void setField(final String name, final Object value) {
			switch (name) {
				case "type" -> this.type = value;
				case "timestamp" -> this.timestamp = value;
				case "signer" -> this.signer = value;
				case "recipient" -> this.recipient = value;
				case "message" -> this.message = value;
				default -> super.setField(name, value);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "type" -> this.type;
				case "timestamp" -> this.timestamp;
				case "signer" -> this.signer;
				case "recipient" -> this.recipient;
				case "message" -> this.message;
				default -> super.getField(name);
			};
		}
	}

	private static final class BagWithExtras extends Bag {
		Object foo;

		@Override
		public void setField(final String name, final Object value) {
			if ("foo".equals(name))
				this.foo = value;
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "foo".equals(name) ? this.foo : super.getField(name);
		}
	}

	private static final class BagWithFee extends Bag {
		Object fee;
		Object deadline;

		@Override
		public void setField(final String name, final Object value) {
			switch (name) {
				case "fee" -> this.fee = value;
				case "deadline" -> this.deadline = value;
				default -> super.setField(name, value);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "fee" -> this.fee;
				case "deadline" -> this.deadline;
				default -> super.getField(name);
			};
		}
	}

	private static final class BagWithMosaics extends CatbufferType {
		Object type;
		Object signer;
		Object mosaics;

		@Override
		public int size() {
			return 0;
		}

		@Override
		protected void serializeInto(final org.symbol.sdk.utils.Writer buffer) {
		}

		@Override
		public Object toJson() {
			return java.util.Map.of();
		}

		@Override
		public void setField(final String name, final Object value) {
			switch (name) {
				case "type" -> this.type = value;
				case "signer" -> this.signer = value;
				case "mosaics" -> this.mosaics = value;
				default -> super.setField(name, value);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "type" -> this.type;
				case "signer" -> this.signer;
				case "mosaics" -> this.mosaics;
				default -> super.getField(name);
			};
		}
	}

	private static Map<String, Object> baseDescriptor() {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("type", "transfer");
		descriptor.put("timestamp", 12345);
		descriptor.put("signer", "signerName");
		descriptor.put("recipient", "recipientName");
		descriptor.put("message", "hello world");
		return descriptor;
	}

	private static TransactionDescriptorProcessor createProcessor(final Map<String, Object> extra) {
		final Map<String, Object> descriptor = baseDescriptor();
		if (null != extra)
			descriptor.putAll(extra);

		final Map<String, Function<Object, Object>> typeParsingRules = new HashMap<>();
		typeParsingRules.put("PublicKey", name -> name + " PUBLICKEY");

		final TransactionDescriptorProcessor processor = new TransactionDescriptorProcessor(descriptor, typeParsingRules, null);
		final Map<String, String> hints = new HashMap<>();
		hints.put("signer", "PublicKey");
		hints.put("timestamp", "Number");
		processor.setTypeHints(hints);
		return processor;
	}

	private static TransactionDescriptorProcessor createProcessor() {
		return createProcessor(null);
	}

	private static TransactionDescriptorProcessor createProcessorWithConverter(final Object deadlineValue) {
		final Map<String, Object> descriptor = baseDescriptor();
		descriptor.put("fee", 100);
		descriptor.put("deadline", null == deadlineValue ? Integer.valueOf(300) : deadlineValue);

		final Map<String, Function<Object, Object>> typeParsingRules = new HashMap<>();
		typeParsingRules.put("Number", value -> ((Number) value).intValue() + 42);

		final Function<Object, Object> typeConverter = value -> value instanceof Number n ? n.intValue() * 2 : value;

		final TransactionDescriptorProcessor processor = new TransactionDescriptorProcessor(descriptor, typeParsingRules, typeConverter);
		final Map<String, String> hints = new HashMap<>();
		hints.put("timestamp", "Number");
		processor.setTypeHints(hints);
		return processor;
	}

	private static void assertCannotLookupValueWhenDescriptorDoesNotContainKey(
			final java.util.function.Supplier<TransactionDescriptorProcessor> factory) {
		// Arrange:
		final TransactionDescriptorProcessor processor = factory.get();

		// Act:
		final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> processor.lookupValue("foo"));

		// Assert:
		assertThat(ex.getMessage(), containsString("transaction descriptor does not have attribute"));
	}

	// endregion

	// region lookupValue

	@Nested
	class LookupValue {
		@Test
		void cannotLookupValueWhenDescriptorDoesNotContainKey() {
			assertCannotLookupValueWhenDescriptorDoesNotContainKey(() -> createProcessor());
		}

		@Test
		void cannotLookupValueWhenDescriptorDoesNotContainKeyWithConverter() {
			assertCannotLookupValueWhenDescriptorDoesNotContainKey(() -> createProcessorWithConverter(null));
		}

		@Test
		void canLookupValueWithoutTypeHint() {
			// Act:
			final Object actual = createProcessor().lookupValue("message");

			// Assert:
			assertThat(actual, equalTo("hello world"));
		}

		@Test
		void canLookupValueWithoutConversion() {
			// Act:
			final Object actual = createProcessorWithConverter(null).lookupValue("message");

			// Assert:
			assertThat(actual, equalTo("hello world"));
		}

		@Test
		void canLookupValueWithTypeHintButWithoutCustomRule() {
			// Act:
			final Object actual = createProcessor().lookupValue("timestamp");

			// Assert:
			assertThat(actual, equalTo(12345));
		}

		@Test
		void canLookupValueWhenHintsAreAppliedBeforeConversion() {
			// Act:
			// (12345 + 42) * 2
			final Object actual = createProcessorWithConverter(null).lookupValue("timestamp");

			// Assert:
			assertThat(actual, equalTo(24774));
		}

		@Test
		void canLookupValueWithTypeHintAndWithCustomRule() {
			// Act:
			final Object actual = createProcessor().lookupValue("signer");

			// Assert:
			assertThat(actual, equalTo("signerName PUBLICKEY"));
		}

		@Test
		void canLookupValueWhenApplyingConverterToAllFields() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessorWithConverter(null);

			// Act:
			final Object fee = processor.lookupValue("fee");
			final Object deadline = processor.lookupValue("deadline");

			// Assert:
			assertThat(fee, equalTo(200));
			assertThat(deadline, equalTo(600));
		}

		@Test
		void canLookupValueWhenApplyingConverterToAllArrayElements() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessorWithConverter(Arrays.asList(100, 300, 600));

			// Act:
			final Object actual = processor.lookupValue("deadline");

			// Assert:
			assertThat(actual, equalTo(Arrays.asList(200, 600, 1200)));
		}

		@Test
		void canLookupValueWithZeroValue() {
			// Act:
			final Object actual = createProcessorWithConverter(0).lookupValue("deadline");

			// Assert:
			assertThat(actual, equalTo(0));
		}
	}

	// endregion

	// region copyTo

	@Nested
	class CopyTo {
		@Test
		void cannotCopyToWhenDescriptorContainsFieldsNotInTransaction() {
			// Arrange:
			// Bag is missing the 'foo' attribute we'll add to descriptor
			final Map<String, Object> extra = new LinkedHashMap<>();
			extra.put("foo", 1);
			final TransactionDescriptorProcessor processor = createProcessor(extra);
			final Bag bag = new Bag();

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> processor.copyTo(bag));

			// Assert:
			assertThat(ex.getMessage(), containsString("no field"));
		}

		@Test
		void cannotCopyWhenSetFieldRejectsValueShape() {
			// Arrange:
			// copyTo wraps setField's ClassCastException into a descriptive InvalidDescriptorException
			final class TypedBag extends CatbufferType {
				@Override
				public int size() {
					return 0;
				}

				@Override
				protected void serializeInto(final org.symbol.sdk.utils.Writer buffer) {
				}

				@Override
				public Object toJson() {
					return java.util.Map.of();
				}

				@Override
				public void setField(final String name, final Object value) {
					if ("amount".equals(name)) {
						Integer.class.cast(value); // throws ClassCastException for non-Integer
						return;
					}

					super.setField(name, value);
				}
			}

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("amount", "not-a-number");
			final TransactionDescriptorProcessor processor = new TransactionDescriptorProcessor(descriptor, new HashMap<>(), null);

			// Act:
			final InvalidDescriptorException ex = assertThrows(InvalidDescriptorException.class, () -> processor.copyTo(new TypedBag()));

			// Assert:
			assertThat(ex.getMessage(), containsString("cannot set field amount"));
			assertThat(ex.getMessage(), containsString("incompatible"));
		}

		@Test
		void cannotCopyWhenLookupValueParseFails() {
			// Arrange: a type converter that rejects the raw value (as PublicKey.parse would on a bad hex string)
			// throws while lookupValue applies it — copyTo must wrap it, not leak the raw exception.
			final class AcceptAllBag extends CatbufferType {
				@Override
				public int size() {
					return 0;
				}

				@Override
				protected void serializeInto(final org.symbol.sdk.utils.Writer buffer) {
				}

				@Override
				public Object toJson() {
					return java.util.Map.of();
				}

				@Override
				public void setField(final String name, final Object value) {
				}
			}

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("signerPublicKey", "not-hex");
			final TransactionDescriptorProcessor processor = new TransactionDescriptorProcessor(descriptor, new HashMap<>(), value -> {
				throw new IllegalArgumentException("cannot parse");
			});

			// Act: the failure is in lookupValue (the type converter), before setField.
			final InvalidDescriptorException ex = assertThrows(InvalidDescriptorException.class,
					() -> processor.copyTo(new AcceptAllBag()));

			// Assert: the raw IllegalArgumentException is surfaced as a descriptive InvalidDescriptorException.
			assertThat(ex.getMessage(), containsString("cannot set field signerPublicKey"));
		}

		@Test
		void cannotCopyWhenDescriptorContainsComputedField() {
			// Arrange:
			final Map<String, Object> extra = new LinkedHashMap<>();
			extra.put("messageEnvelopeSizeComputed", 123);
			final TransactionDescriptorProcessor processor = createProcessor(extra);
			final Bag bag = new Bag();

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> processor.copyTo(bag));

			// Assert:
			assertThat(ex.getMessage(), containsString("cannot explicitly set computed field"));
		}

		@Test
		void canCopyToWhenTransactionContainsExactFieldsInDescriptor() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessor();
			final Bag bag = new Bag();

			// Act:
			processor.copyTo(bag);

			// Assert:
			assertThat(bag.type, equalTo("transfer"));
			assertThat(bag.timestamp, equalTo(12345));
			assertThat(bag.signer, equalTo("signerName PUBLICKEY"));
			assertThat(bag.recipient, equalTo("recipientName"));
			assertThat(bag.message, equalTo("hello world"));
		}

		@Test
		void canCopyToWhenTransactionContainsFieldsNotInDescriptor() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessor();
			final BagWithExtras bag = new BagWithExtras();

			// Act:
			processor.copyTo(bag);

			// Assert:
			assertThat(bag.type, equalTo("transfer"));
			assertThat(bag.timestamp, equalTo(12345));
			assertThat(bag.signer, equalTo("signerName PUBLICKEY"));
			assertThat(bag.recipient, equalTo("recipientName"));
			assertThat(bag.message, equalTo("hello world"));
			assertThat(bag.foo, equalTo(null));
		}

		@Test
		void canCopyToWhenIgnoreKeysIsNotEmpty() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessor();
			final Bag bag = new Bag();

			// Act:
			processor.copyTo(bag, List.of("type", "recipient"));

			// Assert:
			assertThat(bag.type, equalTo(null));
			assertThat(bag.timestamp, equalTo(12345));
			assertThat(bag.signer, equalTo("signerName PUBLICKEY"));
			assertThat(bag.recipient, equalTo(null));
			assertThat(bag.message, equalTo("hello world"));
		}

		@Test
		void canCopyToWhenTransactionContainsIterableAttribute() {
			// Arrange:
			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", "transfer");
			descriptor.put("signer", "signerName");
			descriptor.put("mosaics", Arrays.asList(Arrays.asList(1, 2), Arrays.asList(3, 5)));

			final Map<String, Function<Object, Object>> typeParsingRules = new HashMap<>();
			typeParsingRules.put("PublicKey", name -> name + " PUBLICKEY");

			final TransactionDescriptorProcessor processor = new TransactionDescriptorProcessor(descriptor, typeParsingRules, null);
			processor.setTypeHints(Map.of("signer", "PublicKey"));

			final BagWithMosaics bag = new BagWithMosaics();

			// Act:
			processor.copyTo(bag);

			// Assert:
			assertThat(bag.type, equalTo("transfer"));
			assertThat(bag.signer, equalTo("signerName PUBLICKEY"));
			assertThat(bag.mosaics, equalTo(Arrays.asList(Arrays.asList(1, 2), Arrays.asList(3, 5))));
		}

		@Test
		void canCopyToWithCustomConverter() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessorWithConverter(null);
			final BagWithFee bag = new BagWithFee();

			// Act:
			processor.copyTo(bag);

			// Assert:
			assertThat(bag.type, equalTo("transfer"));
			assertThat(bag.timestamp, equalTo(24774));
			assertThat(bag.signer, equalTo("signerName"));
			assertThat(bag.recipient, equalTo("recipientName"));
			assertThat(bag.message, equalTo("hello world"));
			assertThat(bag.fee, equalTo(200));
			assertThat(bag.deadline, equalTo(600));
		}
	}

	// endregion

	// region setTypeHints

	@Nested
	class SetTypeHints {
		@Test
		void canChangeTypeHints() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessor();
			final Bag bag = new Bag();

			// Act:
			processor.setTypeHints(Map.of("recipient", "PublicKey"));
			processor.copyTo(bag);

			// Assert:
			assertThat(bag.type, equalTo("transfer"));
			assertThat(bag.timestamp, equalTo(12345));
			assertThat(bag.signer, equalTo("signerName"));
			assertThat(bag.recipient, equalTo("recipientName PUBLICKEY"));
			assertThat(bag.message, equalTo("hello world"));
		}

		@Test
		void canClearTypeHints() {
			// Arrange:
			final TransactionDescriptorProcessor processor = createProcessor();
			final Bag bag = new Bag();

			// Act:
			processor.setTypeHints(null);
			processor.copyTo(bag);

			// Assert:
			assertThat(bag.type, equalTo("transfer"));
			assertThat(bag.timestamp, equalTo(12345));
			assertThat(bag.signer, equalTo("signerName"));
			assertThat(bag.recipient, equalTo("recipientName"));
			assertThat(bag.message, equalTo("hello world"));
		}
	}

	// endregion
}
