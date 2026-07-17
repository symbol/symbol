package org.symbol.sdk;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

/**
 * Processes a transaction descriptor: transforms each raw value through hint-driven parsing rules and pushes the result onto a
 * {@link CatbufferType} entity via its generated {@code setField} switch. Created on demand by {@link RuleBasedTransactionFactory}.
 */
public final class TransactionDescriptorProcessor {

	private final Map<String, Object> transactionDescriptor;

	private final Map<String, Function<Object, Object>> typeParsingRules;

	private final Function<Object, Object> typeConverter;

	private Map<String, String> typeHints = Collections.emptyMap();

	/**
	 * Creates a transaction descriptor processor.
	 *
	 * @param transactionDescriptor Transaction descriptor.
	 * @param typeParsingRules Type-dependent parsing rules (hint name → transform).
	 * @param typeConverter Convert a generated type to an SDK type, may be {@code null}.
	 */
	TransactionDescriptorProcessor(final Map<String, Object> transactionDescriptor,
			final Map<String, Function<Object, Object>> typeParsingRules, final Function<Object, Object> typeConverter) {
		this.transactionDescriptor = transactionDescriptor;
		this.typeParsingRules = typeParsingRules;
		this.typeConverter = null != typeConverter ? typeConverter : value -> value;
	}

	private Object lookupValueAndApplyTypeHints(final String key) {
		if (!transactionDescriptor.containsKey(key))
			throw new InvalidDescriptorException("transaction descriptor does not have attribute " + key);

		Object value = transactionDescriptor.get(key);
		final String typeHint = typeHints.get(key);
		final Function<Object, Object> rule = null != typeHint ? typeParsingRules.get(typeHint) : null;
		if (null != rule)
			value = rule.apply(value);

		return value;
	}

	/**
	 * Looks up the value for {@code key} and converts it; {@link List} values are converted element-wise.
	 *
	 * @param key Key for which to retrieve a value.
	 * @return Value corresponding to {@code key}.
	 */
	Object lookupValue(final String key) {
		final Object value = lookupValueAndApplyTypeHints(key);
		if (value instanceof List<?> list) {
			return list.stream().map(typeConverter).collect(java.util.stream.Collectors.toCollection(java.util.ArrayList::new));
		}

		return typeConverter.apply(value);
	}

	/**
	 * Copies descriptor information to a transaction with an ignore list.
	 *
	 * @param transaction Transaction to which to copy keys.
	 * @param ignoreKeys Keys of descriptor values not to copy. May be {@code null}.
	 */
	void copyTo(final CatbufferType transaction, final List<String> ignoreKeys) {
		for (String key : transactionDescriptor.keySet()) {
			if (null != ignoreKeys && ignoreKeys.contains(key))
				continue;

			if (key.endsWith("Computed"))
				throw new InvalidDescriptorException("cannot explicitly set computed field " + key);

			try {
				transaction.setField(key, lookupValue(key));
			} catch (final InvalidDescriptorException ex) {
				throw ex;
			} catch (final ClassCastException | IllegalArgumentException ex) {
				throw new InvalidDescriptorException(String.format("cannot set field %s on %s: incompatible value (%s)", key,
						transaction.getClass().getName(), ex.getMessage()), ex);
			}
		}
	}

	/**
	 * Copies all descriptor information to a transaction.
	 *
	 * @param transaction Transaction to which to copy keys.
	 */
	void copyTo(final CatbufferType transaction) {
		copyTo(transaction, null);
	}

	/**
	 * Sets the type hints used during {@link #lookupValue(String)}.
	 *
	 * @param typeHints New type hints. May be {@code null} to clear.
	 */
	void setTypeHints(final Map<String, String> typeHints) {
		this.typeHints = null != typeHints ? typeHints : Collections.emptyMap();
	}
}
