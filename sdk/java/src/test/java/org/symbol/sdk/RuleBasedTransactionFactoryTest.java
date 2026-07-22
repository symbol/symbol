package org.symbol.sdk;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.instanceOf;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.sameInstance;
import static org.junit.jupiter.api.Assertions.assertThrows;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

import org.junit.jupiter.api.Nested;
import org.junit.jupiter.api.Test;

/**
 * Nested classes mirror the describe blocks in the JS reference {@code RuleBasedTransactionFactory_spec.js}.
 */
final class RuleBasedTransactionFactoryTest {
	private static final String PUBLIC_KEY_HEX = "364F3694A022DB4DC59558944707C6679F6FD7E1A7B99CDE8F7D16D3FF515D28";

	private static Map<String, Object> mosaicDescriptor() {
		final Map<String, Object> descriptor = new LinkedHashMap<>();
		descriptor.put("mosaicId", 0x0123456789ABCDEFL);
		descriptor.put("amount", 123456789123456789L);
		return descriptor;
	}

	// region mock module

	// Test stand-ins mirroring the generated shapes: pods carry static parse(Object), structs
	// carry the setField/getField switch the descriptor processor dispatches through.

	public static final class MosaicId extends BaseValue<MosaicId> {
		public static final int SIZE = 8;
		public static final boolean IS_SIGNED = false;

		public MosaicId(final long value) {
			super(value, SIZE, IS_SIGNED);
		}

		public MosaicId() {
			this(0L);
		}

		public static MosaicId parse(final Object descriptorValue) {
			if (descriptorValue instanceof MosaicId typed)
				return typed;

			if (descriptorValue instanceof String string)
				return new MosaicId(org.symbol.sdk.utils.Converter.toLong(string));

			return new MosaicId(((Number) descriptorValue).longValue());
		}
	}

	public static final class Amount extends BaseValue<Amount> {
		public static final int SIZE = 8;
		public static final boolean IS_SIGNED = false;

		public Amount(final long value) {
			super(value, SIZE, IS_SIGNED);
		}

		public Amount() {
			this(0L);
		}

		public static Amount parse(final Object descriptorValue) {
			if (descriptorValue instanceof Amount typed)
				return typed;

			if (descriptorValue instanceof String string)
				return new Amount(org.symbol.sdk.utils.Converter.toLong(string));

			return new Amount(((Number) descriptorValue).longValue());
		}
	}

	public static final class SigningPublicKey extends ByteArray {
		public static final int SIZE = 32;

		public SigningPublicKey(final byte[] bytes) {
			super(bytes, SIZE);
		}

		public SigningPublicKey(final String hex) {
			this(org.symbol.sdk.utils.Converter.hexToUint8(hex));
		}

		public static SigningPublicKey parse(final Object descriptorValue) {
			if (descriptorValue instanceof SigningPublicKey typed)
				return typed;

			if (descriptorValue instanceof String hex)
				return new SigningPublicKey(hex);

			return new SigningPublicKey(ByteArray.toBytes(descriptorValue));
		}
	}

	public static final class Hash256 extends ByteArray {
		public static final int SIZE = 32;

		public Hash256(final byte[] bytes) {
			super(bytes, SIZE);
		}

		public Hash256(final String hex) {
			this(org.symbol.sdk.utils.Converter.hexToUint8(hex));
		}

		public static Hash256 parse(final Object descriptorValue) {
			if (descriptorValue instanceof Hash256 typed)
				return typed;

			if (descriptorValue instanceof String hex)
				return new Hash256(hex);

			return new Hash256(ByteArray.toBytes(descriptorValue));
		}
	}

	public enum NetworkType implements Serializer {
		MAINNET(104),
		TESTNET(152);

		private final int value;

		NetworkType(final int value) {
			this.value = value;
		}

		public int getValue() {
			return value;
		}

		public static NetworkType fromValue(final int value) {
			for (NetworkType candidate : values()) {
				if (candidate.value == value)
					return candidate;
			}
			throw new IllegalArgumentException("invalid enum value " + value);
		}

		public static NetworkType parse(final Object descriptorValue) {
			if (descriptorValue instanceof NetworkType typed)
				return typed;

			if (descriptorValue instanceof String name) {
				try {
					return valueOf(name.toUpperCase(java.util.Locale.ROOT));
				} catch (final IllegalArgumentException ex) {
					throw new IllegalArgumentException("unknown value " + name + " for type NetworkType", ex);
				}
			}

			if (descriptorValue instanceof Number number)
				return fromValue(org.symbol.sdk.utils.Converter.toInt(number));

			throw new IllegalArgumentException("cannot parse " + descriptorValue.getClass().getName() + " into NetworkType");
		}

		@Override
		public int size() {
			return 1;
		}

		@Override
		public byte[] serialize() {
			return new byte[]{
					(byte) value
			};
		}
	}

	public static final class MosaicFlags {
		public static final MosaicFlags NONE = new MosaicFlags(0);
		public static final MosaicFlags SUPPLY_MUTABLE = new MosaicFlags(1);
		public static final MosaicFlags TRANSFERABLE = new MosaicFlags(2);
		public static final MosaicFlags RESTRICTABLE = new MosaicFlags(4);
		public static final MosaicFlags REVOKABLE = new MosaicFlags(8);

		public final int value;

		public MosaicFlags(final int value) {
			this.value = value;
		}

		public static MosaicFlags or(final MosaicFlags... flagsArray) {
			int combined = 0;
			for (final MosaicFlags flags : flagsArray)
				combined |= flags.value;

			return new MosaicFlags(combined);
		}

		// mirrors the generated flags parse arms: instance / space-separated names / number, with the generated error messages
		public static MosaicFlags parse(final Object rawValue) {
			if (rawValue instanceof MosaicFlags typed)
				return typed;

			if (rawValue instanceof String names) {
				int combined = 0;
				for (final String name : names.split(" "))
					combined |= fromName(name).value;

				return new MosaicFlags(combined);
			}

			if (rawValue instanceof Number number)
				return new MosaicFlags(org.symbol.sdk.utils.Converter.toInt(number));

			throw new IllegalArgumentException("cannot parse " + rawValue.getClass().getName() + " into MosaicFlags");
		}

		private static MosaicFlags fromName(final String name) {
			return switch (name.toLowerCase(java.util.Locale.ROOT)) {
				case "none" -> NONE;
				case "supply_mutable" -> SUPPLY_MUTABLE;
				case "transferable" -> TRANSFERABLE;
				case "restrictable" -> RESTRICTABLE;
				case "revokable" -> REVOKABLE;
				default -> throw new IllegalArgumentException("unknown value " + name + " for type MosaicFlags");
			};
		}

		@Override
		public boolean equals(final Object other) {
			return other instanceof MosaicFlags flags && this.value == flags.value;
		}

		@Override
		public int hashCode() {
			return Integer.hashCode(this.value);
		}
	}

	private abstract static class TestStruct extends CatbufferType {
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
	}

	public static final class UnresolvedMosaic extends TestStruct {
		public MosaicId mosaicId = new MosaicId();
		public Amount amount = new Amount();

		@Override
		public Map<String, String> typeHints() {
			return Map.of("mosaicId", "pod:MosaicId", "amount", "pod:Amount");
		}

		@Override
		public void setField(final String name, final Object value) {
			switch (name) {
				case "mosaicId" -> this.mosaicId = (MosaicId) value;
				case "amount" -> this.amount = (Amount) value;
				default -> super.setField(name, value);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "mosaicId" -> this.mosaicId;
				case "amount" -> this.amount;
				default -> super.getField(name);
			};
		}
	}

	public static final class StructPlain extends TestStruct {
		public Object mosaicId = 0L;
		public Object amount = 0L;

		@Override
		public void setField(final String name, final Object value) {
			switch (name) {
				case "mosaicId" -> this.mosaicId = value;
				case "amount" -> this.amount = value;
				default -> super.setField(name, value);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "mosaicId" -> this.mosaicId;
				case "amount" -> this.amount;
				default -> super.getField(name);
			};
		}
	}

	public static final class StructWithBytes extends TestStruct {
		public byte[] mosaicId = new byte[0];
		public byte[] amount = new byte[0];

		@Override
		public void setField(final String name, final Object value) {
			switch (name) {
				case "mosaicId" -> this.mosaicId = asBytes(value);
				case "amount" -> this.amount = asBytes(value);
				default -> super.setField(name, value);
			}
		}

		@Override
		public Object getField(final String name) {
			return switch (name) {
				case "mosaicId" -> this.mosaicId;
				case "amount" -> this.amount;
				default -> super.getField(name);
			};
		}
	}

	// The nested-auto-encode doubles wrap StructWithBytes rather than the JS StructPlain: JS auto-encodes strings via a
	// factory-level recursive walk, while Java coerces inside each byte[] setField arm (CatbufferType.asBytes), so the
	// byte[]-typed inner struct is where the equivalent nested coercion is observable.
	public static final class StructWrapped extends TestStruct {
		public StructWithBytes inner = new StructWithBytes();

		@Override
		public Map<String, String> typeHints() {
			return Map.of("inner", "struct:StructWithBytes");
		}

		@Override
		public void setField(final String name, final Object value) {
			if ("inner".equals(name))
				this.inner = (StructWithBytes) value;
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "inner".equals(name) ? this.inner : super.getField(name);
		}
	}

	public static final class StructAggregate extends TestStruct {
		public List<StructWithBytes> components = new ArrayList<>();

		@Override
		public Map<String, String> typeHints() {
			return Map.of("components", "array[StructWithBytes]");
		}

		@Override
		public void setField(final String name, final Object value) {
			if ("components".equals(name))
				this.components = asList(value, StructWithBytes.class);
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "components".equals(name) ? this.components : super.getField(name);
		}
	}

	public static final class StructArrayMember extends TestStruct {
		public List<MosaicId> mosaicIds = new ArrayList<>();

		@Override
		public Map<String, String> typeHints() {
			return Map.of("mosaicIds", "array[MosaicId]");
		}

		@Override
		public void setField(final String name, final Object value) {
			if ("mosaicIds".equals(name))
				this.mosaicIds = asList(value, MosaicId.class);
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "mosaicIds".equals(name) ? this.mosaicIds : super.getField(name);
		}
	}

	public static final class StructEnumMember extends TestStruct {
		public NetworkType network;

		@Override
		public Map<String, String> typeHints() {
			return Map.of("network", "enum:NetworkType");
		}

		@Override
		public void setField(final String name, final Object value) {
			if ("network".equals(name))
				this.network = (NetworkType) value;
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "network".equals(name) ? this.network : super.getField(name);
		}
	}

	public static final class StructStructMember extends TestStruct {
		public UnresolvedMosaic mosaic;

		@Override
		public Map<String, String> typeHints() {
			return Map.of("mosaic", "struct:UnresolvedMosaic");
		}

		@Override
		public void setField(final String name, final Object value) {
			if ("mosaic".equals(name))
				this.mosaic = (UnresolvedMosaic) value;
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "mosaic".equals(name) ? this.mosaic : super.getField(name);
		}
	}

	public static final class StructHashMember extends TestStruct {
		public Hash256 hash;

		// non-prefixed hint: no rule applies, so the raw SDK ByteArray reaches setField, whose asByteArray bridges it to the same-named
		// model twin (CryptoTypes.Hash256 -> this Hash256)
		@Override
		public Map<String, String> typeHints() {
			return Map.of("hash", "Hash256");
		}

		@Override
		public void setField(final String name, final Object value) {
			if ("hash".equals(name))
				this.hash = asByteArray(value, Hash256.class, Hash256::new);
			else
				super.setField(name, value);
		}

		@Override
		public Object getField(final String name) {
			return "hash".equals(name) ? this.hash : super.getField(name);
		}
	}

	private static Function<Object, Object> requireRule(final RuleBasedTransactionFactory factory, final String name) {
		final Function<Object, Object> rule = factory.rules.get(name);
		if (null == rule)
			throw new AssertionError("no rule with name " + name);

		return rule;
	}

	// endregion

	// region pod parser

	@Nested
	class PodParser {
		@Test
		void canHandleRawValue() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParser("SigningPublicKey", SigningPublicKey::parse);

			// Act:
			final Object parsed = requireRule(factory, "SigningPublicKey").apply(PUBLIC_KEY_HEX);

			// Assert:
			assertThat(parsed, equalTo(new SigningPublicKey(PUBLIC_KEY_HEX)));
		}

		@Test
		void canHandleTypedValue() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParser("SigningPublicKey", SigningPublicKey::parse);
			final SigningPublicKey publicKey = new SigningPublicKey(PUBLIC_KEY_HEX);

			// Act:
			final Object parsed = requireRule(factory, "SigningPublicKey").apply(publicKey);

			// Assert:
			assertThat(parsed, is(publicKey));
		}

		@Test
		void usesTypeRuleOverrideWhenAvailable() {
			// Arrange:
			final Map<String, Function<Object, Object>> overrides = new HashMap<>();
			overrides.put("SigningPublicKey", value -> "pubkey " + value);

			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(null, overrides);
			factory.addPodParser("SigningPublicKey", SigningPublicKey::parse);
			final SigningPublicKey publicKey = new SigningPublicKey(PUBLIC_KEY_HEX);

			// Act:
			final Object parsed = requireRule(factory, "SigningPublicKey").apply(publicKey);

			// Assert:
			assertThat(parsed, equalTo("pubkey " + publicKey));
		}
	}

	// endregion

	// region flags parser

	@Nested
	class FlagsParser {
		private Function<Object, Object> flagsRule() {
			// the generated flags parse is registered as a pod parser (the Java analog of the JS addFlagsParser)
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("MosaicFlags", MosaicFlags::parse));
			return requireRule(factory, "MosaicFlags");
		}

		private void assertFlagsParser(final Object inputValue, final MosaicFlags expectedValue) {
			// Act:
			final Object parsed = flagsRule().apply(inputValue);

			// Assert:
			assertThat(parsed, equalTo(expectedValue));
		}

		@Test
		void canHandleSingleStringFlag() {
			assertFlagsParser("restrictable", MosaicFlags.RESTRICTABLE);
		}

		@Test
		void canHandleMultipleStringFlags() {
			assertFlagsParser("supply_mutable restrictable revokable",
					MosaicFlags.or(MosaicFlags.SUPPLY_MUTABLE, MosaicFlags.RESTRICTABLE, MosaicFlags.REVOKABLE));
		}

		@Test
		void failsIfAnyStringIsUnknown() {
			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
					() -> flagsRule().apply("supply_mutable foo revokable"));

			// Assert:
			assertThat(ex.getMessage(), containsString("unknown value foo for type MosaicFlags"));
		}

		@Test
		void canHandleInts() {
			assertFlagsParser(9, MosaicFlags.or(MosaicFlags.SUPPLY_MUTABLE, MosaicFlags.REVOKABLE));
		}

		@Test
		void passesNonParsedValuesAsIs() {
			// Arrange:
			final MosaicFlags value = MosaicFlags.or(MosaicFlags.SUPPLY_MUTABLE, MosaicFlags.RESTRICTABLE, MosaicFlags.REVOKABLE);

			// Act:
			final Object parsed = flagsRule().apply(value);

			// Assert: a typed instance passes through unchanged
			assertThat(parsed, sameInstance(value));

			// deliberate divergence: the JS reflective parser passes unrecognized shapes (fractional numbers, lists)
			// through as-is; the generated Java parse is strict and rejects them instead
			assertThrows(IllegalArgumentException.class, () -> flagsRule().apply(1.2));
			assertThrows(IllegalArgumentException.class, () -> flagsRule().apply(List.of(1, 2, 3, 4)));
		}
	}

	// endregion

	// region enum parser

	@Nested
	class EnumParser {
		private Function<Object, Object> enumRule() {
			// the generated enum parse is registered as a pod parser (the Java analog of the JS addEnumParser)
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("NetworkType", NetworkType::parse));
			return requireRule(factory, "NetworkType");
		}

		@Test
		void canHandleString() {
			assertThat(enumRule().apply("testnet"), equalTo(NetworkType.TESTNET));
		}

		@Test
		void failsIfStringIsUnknown() {
			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> enumRule().apply("Bitcoin"));

			// Assert:
			assertThat(ex.getMessage(), containsString("unknown value Bitcoin for type NetworkType"));
		}

		@Test
		void canHandleInts() {
			assertThat(enumRule().apply(152), equalTo(NetworkType.TESTNET));
		}

		@Test
		void passesNonParsedValuesAsIs() {
			// Act:
			final Object parsed = enumRule().apply(NetworkType.TESTNET);

			// Assert: a typed instance passes through unchanged
			assertThat(parsed, sameInstance(NetworkType.TESTNET));

			// deliberate divergence: the JS reflective parser passes unrecognized shapes (fractional numbers, lists)
			// through as-is; the generated Java parse is strict and rejects them instead
			assertThrows(IllegalArgumentException.class, () -> enumRule().apply(1.2));
			assertThrows(IllegalArgumentException.class, () -> enumRule().apply(List.of(1, 2, 3, 4)));
		}
	}

	// endregion

	// region struct parser

	@Nested
	class StructParser {
		private static final String HASH_HEX = "E9B3AEDE9A57C2B8C3D78DB9805D12AB0D983B63CE8F89D8DFE108D0FF08D23C";

		@Test
		void canParsePlainFields() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructPlain", StructPlain::new);

			final Map<String, Object> descriptor = mosaicDescriptor();

			// Act:
			final StructPlain parsed = (StructPlain) requireRule(factory, "struct:StructPlain").apply(descriptor);

			// Assert:
			assertThat(parsed.mosaicId, equalTo(0x0123456789ABCDEFL));
			assertThat(parsed.amount, equalTo(123456789123456789L));
		}

		@Test
		void canParsePodFields() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));
			factory.addStructParser("UnresolvedMosaic", UnresolvedMosaic::new);

			final Map<String, Object> descriptor = mosaicDescriptor();

			// Act:
			final UnresolvedMosaic parsed = (UnresolvedMosaic) requireRule(factory, "struct:UnresolvedMosaic").apply(descriptor);

			// Assert:
			assertThat(parsed.mosaicId, equalTo(new MosaicId(0x0123456789ABCDEFL)));
			assertThat(parsed.amount, equalTo(new Amount(123456789123456789L)));
			assertThat(parsed.mosaicId, instanceOf(MosaicId.class));
			assertThat(parsed.amount, instanceOf(Amount.class));
		}

		@Test
		void canParseArrayFields() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse));
			factory.addArrayParser("MosaicId");
			factory.addStructParser("StructArrayMember", StructArrayMember::new);

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("mosaicIds", Arrays.asList(0x0123456789ABCDEFL, 0x3456789123456789L));

			// Act:
			final StructArrayMember parsed = (StructArrayMember) requireRule(factory, "struct:StructArrayMember").apply(descriptor);

			// Assert:
			assertThat(parsed.mosaicIds, equalTo(Arrays.asList(new MosaicId(0x0123456789ABCDEFL), new MosaicId(0x3456789123456789L))));
		}

		@Test
		void canParseEnumFields() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("NetworkType", NetworkType::parse));
			factory.addStructParser("StructEnumMember", StructEnumMember::new);

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("network", "testnet");

			// Act:
			final StructEnumMember parsed = (StructEnumMember) requireRule(factory, "struct:StructEnumMember").apply(descriptor);

			// Assert:
			assertThat(parsed.network, is(NetworkType.TESTNET));
		}

		@Test
		void canParseStructFields() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));
			factory.addStructParser("UnresolvedMosaic", UnresolvedMosaic::new);
			factory.addStructParser("StructStructMember", StructStructMember::new);

			final Map<String, Object> nested = mosaicDescriptor();
			final Map<String, Object> descriptor = Map.of("mosaic", nested);

			// Act:
			final StructStructMember parsed = (StructStructMember) requireRule(factory, "struct:StructStructMember").apply(descriptor);

			// Assert:
			assertThat(parsed.mosaic.mosaicId, equalTo(new MosaicId(0x0123456789ABCDEFL)));
			assertThat(parsed.mosaic.amount, equalTo(new Amount(123456789123456789L)));
		}

		@Test
		void canParseWithTypeConverter() {
			// Arrange:
			// converter doubles any Long (null = "not handled"); StructPlain keeps fields Object-typed
			final Function<Object, Object> typeConverter = value -> value instanceof Long l ? l * 2L : null;
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(typeConverter, null);
			factory.addStructParser("StructPlain", StructPlain::new);

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("mosaicId", 100L);
			descriptor.put("amount", 50L);

			// Act:
			final StructPlain parsed = (StructPlain) requireRule(factory, "struct:StructPlain").apply(descriptor);

			// Assert:
			assertThat(parsed.mosaicId, equalTo(200L));
			assertThat(parsed.amount, equalTo(100L));
		}

		private void assertCanParseWithTypeConverterAndAutodetectByteArrays(final Object hashValue) {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructHashMember", StructHashMember::new);

			final Map<String, Object> descriptor = Map.of("hash", hashValue);

			// Act:
			final StructHashMember parsed = (StructHashMember) requireRule(factory, "struct:StructHashMember").apply(descriptor);

			// Assert:
			assertThat(parsed.hash, equalTo(new Hash256(HASH_HEX)));
			assertThat(parsed.hash, instanceOf(Hash256.class));
		}

		@Test
		void canParseWithTypeConverterAndAutodetectByteArraysFromPublicType() {
			// a hand-written SDK Hash256 on a rule-less field is bridged to its same-named model twin by setField's asByteArray coercion
			assertCanParseWithTypeConverterAndAutodetectByteArrays(new CryptoTypes.Hash256(HASH_HEX));
		}

		@Test
		void canParseWithTypeConverterAndAutodetectByteArraysFromModelType() {
			// an already-model Hash256 passes through the same asByteArray coercion untouched
			assertCanParseWithTypeConverterAndAutodetectByteArrays(new Hash256(HASH_HEX));
		}
	}

	// endregion

	// region array parser

	@Nested
	class ArrayParser {
		@Test
		void cannotAddWithUnknownElementType() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class, () -> factory.addArrayParser("NetworkType"));

			// Assert:
			assertThat(ex.getMessage(), containsString("element rule \"NetworkType\" is unknown"));
		}

		@Test
		void canParseEnumArray() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("NetworkType", NetworkType::parse));
			factory.addArrayParser("NetworkType");

			// Act:
			final Object parsed = requireRule(factory, "array[NetworkType]").apply(Arrays.asList("mainnet", 152));

			// Assert:
			assertThat(parsed, equalTo(Arrays.asList(NetworkType.MAINNET, NetworkType.TESTNET)));
		}

		@Test
		void canParseStructArray() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));
			factory.addStructParser("UnresolvedMosaic", UnresolvedMosaic::new);
			factory.addArrayParser("struct:UnresolvedMosaic");

			final Map<String, Object> first = mosaicDescriptor();
			final Map<String, Object> second = new LinkedHashMap<>();
			second.put("mosaicId", 0x89ABCDEF01234567L);
			second.put("amount", 456789123456789123L);

			// Act:
			final List<?> parsed = (List<?>) requireRule(factory, "array[UnresolvedMosaic]").apply(Arrays.asList(first, second));

			// Assert:
			final UnresolvedMosaic firstParsed = (UnresolvedMosaic) parsed.get(0);
			final UnresolvedMosaic secondParsed = (UnresolvedMosaic) parsed.get(1);
			assertThat(firstParsed.mosaicId, equalTo(new MosaicId(0x0123456789ABCDEFL)));
			assertThat(firstParsed.amount, equalTo(new Amount(123456789123456789L)));
			assertThat(secondParsed.mosaicId, equalTo(new MosaicId(0x89ABCDEF01234567L)));
			assertThat(secondParsed.amount, equalTo(new Amount(456789123456789123L)));
		}
	}

	// endregion

	// region autodetect (addPodParsers)

	@Nested
	class Autodetect {
		@Test
		void addPodParsersAddsAllNamedRules() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();

			// Act:
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));

			final Object mosaicId = requireRule(factory, "MosaicId").apply(123L);
			final Object amount = requireRule(factory, "Amount").apply(987L);

			// Assert:
			assertThat(factory.rules.containsKey("MosaicId"), is(true));
			assertThat(factory.rules.containsKey("Amount"), is(true));
			assertThat(mosaicId, equalTo(new MosaicId(123)));
			assertThat(amount, equalTo(new Amount(987)));
		}

		@Test
		void addPodParsersHonorsTypeRuleOverrides() {
			// Arrange:
			final Map<String, Function<Object, Object>> overrides = new HashMap<>();
			overrides.put("Amount", value -> new Amount(10L));

			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(null, overrides);
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));

			// Act:
			final Object amount = requireRule(factory, "Amount").apply(1L);
			final Object mosaicId = requireRule(factory, "MosaicId").apply(1L);

			// Assert: the override replaced the generated Amount rule; MosaicId keeps the generated rule
			assertThat(amount, equalTo(new Amount(10L)));
			assertThat(mosaicId, equalTo(new MosaicId(1L)));
		}

		@Test
		void generatedFactoriesRegistryCoversPodsEnumsAndFlags() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();

			// Act:
			factory.addPodParsers(org.symbol.sdk.symbol.models.Models.POD_FACTORIES);

			// Assert:
			assertThat(factory.rules.containsKey("Amount"), is(true)); // pod
			assertThat(factory.rules.containsKey("TransactionType"), is(true)); // enum
			assertThat(factory.rules.containsKey("MosaicFlags"), is(true)); // flags
			assertThat(requireRule(factory, "TransactionType").apply("transfer"),
					is(org.symbol.sdk.symbol.models.TransactionType.TRANSFER));
		}
	}

	// endregion

	// region create from factory

	@Nested
	class CreateFromFactory {
		@Test
		void canCreateSimpleStruct() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructPlain", StructPlain::new);

			final Function<Object, Object> entityFactory = entityType -> {
				if (Integer.valueOf(123).equals(entityType))
					return new StructPlain();

				throw new AssertionError("unexpected type " + entityType);
			};

			final Map<String, Object> descriptor = mosaicDescriptor();
			descriptor.put("type", 123);

			// Act:
			final StructPlain parsed = (StructPlain) factory.createFromFactory(entityFactory, descriptor);

			// Assert:
			assertThat(parsed.mosaicId, equalTo(0x0123456789ABCDEFL));
			assertThat(parsed.amount, equalTo(123456789123456789L));
		}

		@Test
		void canCreateStructWithNestedRules() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));
			factory.addStructParser("UnresolvedMosaic", UnresolvedMosaic::new);

			final Function<Object, Object> entityFactory = entityType -> {
				if (Integer.valueOf(123).equals(entityType))
					return new UnresolvedMosaic();

				throw new AssertionError("unexpected type " + entityType);
			};

			final Map<String, Object> descriptor = mosaicDescriptor();
			descriptor.put("type", 123);

			// Act:
			final UnresolvedMosaic parsed = (UnresolvedMosaic) factory.createFromFactory(entityFactory, descriptor);

			// Assert:
			assertThat(parsed.mosaicId, equalTo(new MosaicId(0x0123456789ABCDEFL)));
			assertThat(parsed.amount, equalTo(new Amount(123456789123456789L)));
		}

		@Test
		void canCreateStructWithTypeConverter() {
			// Arrange: the converter runs after the pod rules
			final Function<Object, Object> typeConverter = value -> value instanceof Amount amount ? new Amount(2 * amount.value()) : null;
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(typeConverter, null);
			factory.addPodParsers(Map.of("MosaicId", MosaicId::parse, "Amount", Amount::parse));
			factory.addStructParser("UnresolvedMosaic", UnresolvedMosaic::new);

			final Function<Object, Object> entityFactory = entityType -> {
				if (Integer.valueOf(123).equals(entityType))
					return new UnresolvedMosaic();

				throw new AssertionError("unexpected type " + entityType);
			};

			final Map<String, Object> descriptor = mosaicDescriptor();
			descriptor.put("type", 123);

			// Act:
			final UnresolvedMosaic parsed = (UnresolvedMosaic) factory.createFromFactory(entityFactory, descriptor);

			// Assert: mosaicId was not handled by the converter (kept as rule output); amount was rule-parsed then converted
			assertThat(parsed.mosaicId, equalTo(new MosaicId(0x0123456789ABCDEFL)));
			assertThat(parsed.amount, equalTo(new Amount(2 * 123456789123456789L)));
		}

		@Test
		void canCreateStructAndAutoEncodeStrings() {
			// Arrange:
			// String values for byte[] fields are UTF-8 encoded by the asBytes coercion
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructWithBytes", StructWithBytes::new);

			final Function<Object, Object> entityFactory = entityType -> new StructWithBytes();

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", 123);
			descriptor.put("mosaicId", "01234567_89ABCDEF");
			descriptor.put("amount", "123_456_789_123_456_789");

			// Act:
			final StructWithBytes parsed = (StructWithBytes) factory.createFromFactory(entityFactory, descriptor);

			// Assert:
			assertThat(parsed.mosaicId, equalTo("01234567_89ABCDEF".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
			assertThat(parsed.amount, equalTo("123_456_789_123_456_789".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
		}

		@Test
		void canCreateStructAndAutoEncodeNestedStrings() {
			// Arrange: use a wrapped struct but set string values in the inner struct
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructWithBytes", StructWithBytes::new);
			factory.addStructParser("StructWrapped", StructWrapped::new);

			final Function<Object, Object> entityFactory = entityType -> new StructWrapped();

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", 123);
			descriptor.put("inner", Map.of("mosaicId", "01234567_89ABCDEF", "amount", "123_456_789_123_456_789"));

			// Act:
			final StructWrapped parsed = (StructWrapped) factory.createFromFactory(entityFactory, descriptor);

			// Assert: string values were encoded into utf8
			assertThat(parsed.inner.mosaicId, equalTo("01234567_89ABCDEF".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
			assertThat(parsed.inner.amount, equalTo("123_456_789_123_456_789".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
		}

		@Test
		void canCreateStructAndAutoEncodeNestedStringsInArray() {
			// Arrange: use an aggregate struct but set string values in the inner struct
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructWithBytes", StructWithBytes::new);
			factory.addArrayParser("struct:StructWithBytes");
			factory.addStructParser("StructAggregate", StructAggregate::new);

			final Function<Object, Object> entityFactory = entityType -> new StructAggregate();

			final Map<String, Object> descriptor = new LinkedHashMap<>();
			descriptor.put("type", 123);
			descriptor.put("components", List.of(Map.of("mosaicId", "01234567_89ABCDEF", "amount", "123_456_789_123_456_789")));

			// Act:
			final StructAggregate parsed = (StructAggregate) factory.createFromFactory(entityFactory, descriptor);

			// Assert: string values were encoded into utf8
			assertThat(parsed.components.size(), equalTo(1));
			assertThat(parsed.components.get(0).mosaicId, equalTo("01234567_89ABCDEF".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
			assertThat(parsed.components.get(0).amount,
					equalTo("123_456_789_123_456_789".getBytes(java.nio.charset.StandardCharsets.UTF_8)));
		}

		@Test
		void cannotCreateStructWhenDescriptorDoesNotHaveType() {
			// Arrange:
			final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory();
			factory.addStructParser("StructPlain", StructPlain::new);

			final Function<Object, Object> entityFactory = entityType -> new StructPlain();

			final Map<String, Object> descriptor = mosaicDescriptor();

			// Act:
			final IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
					() -> factory.createFromFactory(entityFactory, descriptor));

			// Assert:
			assertThat(ex.getMessage(), containsString("transaction descriptor does not have attribute type"));
		}
	}

	// endregion
}
