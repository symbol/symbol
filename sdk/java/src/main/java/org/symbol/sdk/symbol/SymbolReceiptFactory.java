package org.symbol.sdk.symbol;

import java.util.Map;
import java.util.function.Function;

import org.symbol.sdk.RuleBasedTransactionFactory;
import org.symbol.sdk.symbol.models.*;

/**
 * Factory for creating Symbol receipts. Wraps the catbuffer-generated {@link ReceiptFactory#createByName(String)} with a
 * {@link RuleBasedTransactionFactory} preconfigured with receipt specific parsing rules.
 */
public final class SymbolReceiptFactory {

	private final RuleBasedTransactionFactory factory;

	/**
	 * Creates a factory.
	 */
	public SymbolReceiptFactory() {
		this(null);
	}

	/**
	 * Creates a factory with per-type rule overrides.
	 *
	 * @param typeRuleOverrides Per-rule parser overrides keyed by rule name. May be {@code null}.
	 */
	public SymbolReceiptFactory(final Map<String, Function<Object, Object>> typeRuleOverrides) {
		this.factory = buildRules(typeRuleOverrides);
	}

	/**
	 * Creates a receipt from a receipt descriptor.
	 *
	 * @param receiptDescriptor Receipt descriptor.
	 * @return Newly created receipt.
	 */
	public Receipt create(final Map<String, Object> receiptDescriptor) {
		return (Receipt) factory.createFromFactory(entityType -> ReceiptFactory.createByName((String) entityType), receiptDescriptor);
	}

	/**
	 * Gets rule names with registered hints.
	 *
	 * @return Rule names with registered hints.
	 */
	public java.util.Set<String> getRuleNames() {
		// snapshot, not the live keySet view — HashMap.keySet() writes through to the backing rules map
		return java.util.Set.copyOf(factory.rules.keySet());
	}

	private static RuleBasedTransactionFactory buildRules(final Map<String, Function<Object, Object>> typeRuleOverrides) {
		final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(null, typeRuleOverrides);
		factory.addPodParsers(Models.POD_FACTORIES);

		factory.addStructParser("Mosaic", Mosaic::new);

		factory.addPodParser("Address", AddressRules::resolved);

		return factory;
	}
}
