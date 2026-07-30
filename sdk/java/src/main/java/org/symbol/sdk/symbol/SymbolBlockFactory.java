package org.symbol.sdk.symbol;

import java.util.LinkedHashMap;
import java.util.Map;
import java.util.function.Function;

import org.symbol.sdk.RuleBasedTransactionFactory;
import org.symbol.sdk.symbol.models.*;

/**
 * Factory for creating Symbol blocks. Wraps the catbuffer-generated {@link BlockFactory#createByName(String)} with a
 * {@link RuleBasedTransactionFactory} preconfigured with Symbol-block specific parsing rules.
 */
public final class SymbolBlockFactory {

	private final RuleBasedTransactionFactory factory;

	private final Network network;

	/**
	 * Creates a factory for the specified network.
	 *
	 * @param network Symbol network.
	 */
	public SymbolBlockFactory(final Network network) {
		this(network, null);
	}

	/**
	 * Creates a factory for the specified network with per-type rule overrides.
	 *
	 * @param network Symbol network.
	 * @param typeRuleOverrides Per-rule parser overrides keyed by rule name. May be {@code null}.
	 */
	public SymbolBlockFactory(final Network network, final Map<String, Function<Object, Object>> typeRuleOverrides) {
		this.factory = buildRules(typeRuleOverrides);
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
		// blocks carry only resolved Address fields (e.g. beneficiaryAddress), so no type converter is needed (mirrors
		// SymbolReceiptFactory)
		final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(null, typeRuleOverrides);
		factory.addPodParsers(Models.POD_FACTORIES);

		factory.addStructParser("VrfProof", VrfProof::new);

		factory.addPodParser("Address", AddressRules::resolved);
		return factory;
	}
}
