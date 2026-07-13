package org.symbol.sdk;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Supplier;

/**
 * Rule-based transaction factory: registers named parsing rules (POD / enum / flags / struct / array) and applies them via
 * {@link TransactionDescriptorProcessor}. Rules come from the generated {@code Models.FACTORIES} registry and typed struct suppliers.
 */
public final class RuleBasedTransactionFactory {

	private final Function<Object, Object> typeConverter;

	private final Map<String, Function<Object, Object>> typeRuleOverrides;

	/**
	 * Map of rule names to transform functions
	 */
	public final Map<String, Function<Object, Object>> rules = new HashMap<>();

	/**
	 * Creates a rule-based transaction factory for use with catbuffer generated code.
	 *
	 * @param typeConverter Optional convert to a more appropriate type.
	 * @param typeRuleOverrides Per-rule parser overrides keyed by rule name.
	 */
	public RuleBasedTransactionFactory(final Function<Object, Object> typeConverter,
			final Map<String, Function<Object, Object>> typeRuleOverrides) {
		final Function<Object, Object> converter = typeConverter;
		this.typeConverter = null == converter ? value -> value : value -> {
			final Object converted = converter.apply(value);
			return null != converted ? converted : value;
		};
		this.typeRuleOverrides = null != typeRuleOverrides ? typeRuleOverrides : Map.of();
	}

	/**
	 * Creates a rule-based transaction factory without a custom type converter or overrides.
	 */
	public RuleBasedTransactionFactory() {
		this(null, null);
	}

	/**
	 * Registers a batch of named value-parse rules — typically a generated {@code Models.FACTORIES} registry.
	 *
	 * @param parsers Rule name → parse function.
	 */
	public void registerParsers(final Map<String, Function<Object, Object>> parsers) {
		parsers.forEach(this::addPodParser);
	}

	/**
	 * Registers a parser for a POD type. The parser is replaced by a matching {@code typeRuleOverrides} entry when one was supplied for
	 * {@code name}.
	 *
	 * @param name Rule name (and override key).
	 * @param parser Parse function converting a raw descriptor value into the POD.
	 */
	public void addPodParser(final String name, final Function<Object, Object> parser) {
		rules.put(name, typeRuleOverrides.getOrDefault(name, parser));
	}

	/**
	 * Creates a struct parser (allows nested parsing) over a strongly typed factory reference.
	 *
	 * @param name Class name (rule key).
	 * @param factory Supplier producing a fresh, default-initialized struct instance.
	 */
	public void addStructParser(final String name, final Supplier<? extends CatbufferType> factory) {
		rules.put("struct:" + name, descriptor -> {
			if (!(descriptor instanceof Map<?, ?> rawDescriptor))
				return descriptor;

			final Map<String, Object> structDescriptor = new HashMap<>();
			rawDescriptor.forEach((key, value) -> structDescriptor.put((String) key, value));
			final TransactionDescriptorProcessor processor = createProcessor(structDescriptor);
			final CatbufferType structValue = factory.get();
			processor.setTypeHints(buildTypeHintsMap(structValue));
			processor.copyTo(structValue, List.of("type"));
			return structValue;
		});
	}

	/**
	 * Creates an array type parser based on an existing element type parser.
	 *
	 * @param name Class name (existing rule key).
	 */
	public void addArrayParser(final String name) {
		final Function<Object, Object> elementRule = rules.get(name);
		if (null == elementRule)
			throw new InvalidDescriptorException("cannot create array type parser because element rule \"" + name + "\" is unknown");

		final String elementName = name.startsWith("struct:") ? name.substring("struct:".length()) : name;
		rules.put("array[" + elementName + "]", values -> {
			final List<Object> result = new ArrayList<>();
			for (Object value : (Iterable<?>) values)
				result.add(elementRule.apply(value));
			return result;
		});
	}

	/**
	 * Creates an entity from a descriptor using a factory. The descriptor's required {@code type} key selects the entity; remaining keys
	 * are copied onto it through {@link TransactionDescriptorProcessor#copyTo}.
	 *
	 * @param factory Function that, given the resolved {@code type} value, returns a new entity.
	 * @param descriptor Entity descriptor.
	 * @return Newly created entity.
	 */
	public Object createFromFactory(final Function<Object, Object> factory, final Map<String, Object> descriptor) {
		final TransactionDescriptorProcessor processor = createProcessor(descriptor);
		final Object entityType = processor.lookupValue("type");
		final CatbufferType entity = (CatbufferType) factory.apply(entityType);

		processor.setTypeHints(buildTypeHintsMap(entity));
		processor.copyTo(entity, List.of("type"));
		return entity;
	}

	private TransactionDescriptorProcessor createProcessor(final Map<String, Object> descriptor) {
		return new TransactionDescriptorProcessor(descriptor, rules, typeConverter);
	}

	/**
	 * Projects the struct's {@link CatbufferType#typeHints} map onto rule keys: {@code pod:X} / {@code enum:X} → {@code X},
	 * {@code struct:X} and {@code array[...]} kept as-is; other hints (e.g. {@code bytes_array}) have no rule and are dropped.
	 */
	private static Map<String, String> buildTypeHintsMap(final CatbufferType structValue) {
		final Map<String, String> typeHints = new HashMap<>();
		for (Map.Entry<String, String> entry : structValue.typeHints().entrySet()) {
			final String hint = entry.getValue();
			String ruleName = null;
			if (hint.startsWith("array["))
				ruleName = hint;
			else if (hint.startsWith("enum:"))
				ruleName = hint.substring("enum:".length());
			else if (hint.startsWith("pod:"))
				ruleName = hint.substring("pod:".length());
			else if (hint.startsWith("struct:"))
				ruleName = hint;

			if (null != ruleName)
				typeHints.put(entry.getKey(), ruleName);
		}

		return typeHints;
	}
}
