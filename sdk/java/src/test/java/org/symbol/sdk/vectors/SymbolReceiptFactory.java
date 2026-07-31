package org.symbol.sdk.vectors;

import java.util.Map;

import org.symbol.sdk.RuleBasedTransactionFactory;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.models.*;

/**
 * Factory for creating Symbol receipts. Wraps the catbuffer-generated {@link ReceiptFactory#createByName(String)} with a
 * {@link RuleBasedTransactionFactory} preconfigured with receipt specific parsing rules.
 */
final class SymbolReceiptFactory {

	private final RuleBasedTransactionFactory factory;

	/**
	 * Creates a factory.
	 */
	public SymbolReceiptFactory() {
		this.factory = buildRules();
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

	private static RuleBasedTransactionFactory buildRules() {
		final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(null, null);
		factory.addPodParsers(Models.POD_FACTORIES);

		factory.addStructParser("Mosaic", Mosaic::new);

		factory.addPodParser("Address", SymbolReceiptFactory::resolvedAddress);

		return factory;
	}

	// replicates the package-private symbol AddressRules.resolved rule via public API
	private static org.symbol.sdk.symbol.models.Address resolvedAddress(final Object value) {
		final Address address = Address.parse(value);
		if (address.isAlias())
			throw new IllegalArgumentException("resolved address field cannot be a namespace alias: " + address);

		return new org.symbol.sdk.symbol.models.Address(address.bytes());
	}
}
