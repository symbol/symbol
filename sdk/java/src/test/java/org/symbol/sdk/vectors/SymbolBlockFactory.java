package org.symbol.sdk.vectors;

import java.util.LinkedHashMap;
import java.util.Map;

import org.symbol.sdk.RuleBasedTransactionFactory;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.Network;
import org.symbol.sdk.symbol.models.*;

/**
 * Factory for creating Symbol blocks. Wraps the catbuffer-generated {@link BlockFactory#createByName(String)} with a
 * {@link RuleBasedTransactionFactory} preconfigured with Symbol-block specific parsing rules.
 */
final class SymbolBlockFactory {

	private final RuleBasedTransactionFactory factory;

	private final Network network;

	/**
	 * Creates a factory for the specified network.
	 *
	 * @param network Symbol network.
	 */
	public SymbolBlockFactory(final Network network) {
		this.factory = buildRules();
		this.network = network;
	}

	/**
	 * Creates a block from a block descriptor. A {@code transactions} value must be a list of built {@link Transaction} model instances —
	 * nested transaction descriptor maps are not parsed here.
	 *
	 * @param blockDescriptor Block descriptor.
	 * @return Newly created block.
	 */
	public Block create(final Map<String, Object> blockDescriptor) {
		final Map<String, Object> descriptorWithNetwork = new LinkedHashMap<>(blockDescriptor);
		descriptorWithNetwork.put("network", Byte.toUnsignedInt(network.identifier));

		return (Block) factory.createFromFactory(entityType -> BlockFactory.createByName((String) entityType), descriptorWithNetwork);
	}

	private static RuleBasedTransactionFactory buildRules() {
		// blocks carry only resolved Address fields (e.g. beneficiaryAddress), so no type converter is needed (mirrors
		// SymbolReceiptFactory)
		final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(null, null);
		factory.addPodParsers(Models.POD_FACTORIES);

		factory.addStructParser("VrfProof", VrfProof::new);

		factory.addPodParser("Address", SymbolBlockFactory::resolvedAddress);
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
