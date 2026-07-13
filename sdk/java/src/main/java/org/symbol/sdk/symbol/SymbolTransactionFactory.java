package org.symbol.sdk.symbol;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.function.BiConsumer;
import java.util.function.Function;

import org.symbol.sdk.CatbufferType;
import org.symbol.sdk.CryptoTypes.PublicKey;
import org.symbol.sdk.CryptoTypes.Signature;
import org.symbol.sdk.InvalidDescriptorException;
import org.symbol.sdk.RuleBasedTransactionFactory;
import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.Converter;

/**
 * Factory for creating Symbol transactions. Wraps the catbuffer-generated {@link TransactionFactory#createByName(String)} with a
 * {@link RuleBasedTransactionFactory} preconfigured with Symbol specific parsing rules.
 */
public final class SymbolTransactionFactory {

	private final RuleBasedTransactionFactory factory;

	private final Network network;

	/**
	 * Creates a factory for the specified network.
	 *
	 * @param network Symbol network.
	 */
	public SymbolTransactionFactory(final Network network) {
		this(network, null);
	}

	/**
	 * Creates a factory for the specified network with per-type rule overrides.
	 *
	 * @param network Symbol network.
	 * @param typeRuleOverrides Per-rule parser overrides keyed by rule name. May be {@code null}.
	 */
	public SymbolTransactionFactory(final Network network, final Map<String, Function<Object, Object>> typeRuleOverrides) {
		this.factory = buildRules(typeRuleOverrides);
		this.network = network;
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

	/**
	 * Looks up the friendly name for the specified transaction.
	 *
	 * @param transactionType Transaction type.
	 * @param transactionVersion Transaction version.
	 * @return Transaction friendly name (e.g. {@code "transfer_transaction_v1"}).
	 */
	public static String lookupTransactionName(final TransactionType transactionType, final int transactionVersion) {
		return transactionType.name().toLowerCase(Locale.ROOT) + "_transaction_v" + transactionVersion;
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 *
	 * @param transactionDescriptor Transaction descriptor.
	 * @return Newly created transaction.
	 */
	public Transaction create(final Map<String, Object> transactionDescriptor) {
		return create(transactionDescriptor, true);
	}

	/**
	 * Creates a transaction from a transaction descriptor.
	 *
	 * @param transactionDescriptor Transaction descriptor.
	 * @param autosort When {@code true}, descriptor arrays requiring ordering will be automatically sorted.
	 * @return Newly created transaction.
	 */
	public Transaction create(final Map<String, Object> transactionDescriptor, final boolean autosort) {
		return (Transaction) createAndExtend(transactionDescriptor, autosort, TransactionFactory::createByName);
	}

	/**
	 * Creates an embedded transaction from a transaction descriptor.
	 *
	 * @param transactionDescriptor Transaction descriptor.
	 * @return Newly created embedded transaction.
	 */
	public EmbeddedTransaction createEmbedded(final Map<String, Object> transactionDescriptor) {
		return createEmbedded(transactionDescriptor, true);
	}

	/**
	 * Creates an embedded transaction from a transaction descriptor.
	 *
	 * @param transactionDescriptor Transaction descriptor.
	 * @param autosort When {@code true}, descriptor arrays requiring ordering will be automatically sorted.
	 * @return Newly created embedded transaction.
	 */
	public EmbeddedTransaction createEmbedded(final Map<String, Object> transactionDescriptor, final boolean autosort) {
		return (EmbeddedTransaction) createAndExtend(transactionDescriptor, autosort, EmbeddedTransactionFactory::createByName);
	}

	private CatbufferType createAndExtend(final Map<String, Object> transactionDescriptor, final boolean autosort,
			final Function<String, ?> entityFactory) {
		final Map<String, Object> descriptorWithNetwork = new LinkedHashMap<>(transactionDescriptor);
		descriptorWithNetwork.put("network", Byte.toUnsignedInt(network.identifier));

		final CatbufferType target = (CatbufferType) factory.createFromFactory(entityType -> entityFactory.apply((String) entityType),
				descriptorWithNetwork);

		if (autosort)
			target.sort();

		// compute the transaction's artifact id, if its type needs one (data-driven, see ID_ASSIGNERS)
		final BiConsumer<Network, CatbufferType> idAssigner = ID_ASSIGNERS.get((TransactionType) target.getField("type"));
		if (null != idAssigner)
			idAssigner.accept(network, target);

		return target;
	}

	// post-processors that derive a transaction's artifact id after it is built, keyed by transaction type so a new
	// artifact-bearing type is a data addition here rather than another branch in createAndExtend
	private static final Map<TransactionType, BiConsumer<Network, CatbufferType>> ID_ASSIGNERS = Map.of(
			TransactionType.NAMESPACE_REGISTRATION, SymbolTransactionFactory::assignNamespaceId, TransactionType.MOSAIC_DEFINITION,
			SymbolTransactionFactory::assignMosaicId);

	private static void assignNamespaceId(final Network network, final CatbufferType target) {
		final NamespaceRegistrationType registrationType = (NamespaceRegistrationType) target.getField("registrationType");
		final long parentValue;
		if (NamespaceRegistrationType.CHILD == registrationType) {
			// a child registration is parented; fail fast rather than silently deriving a root-style id (which would NPE later
			// in serialize when the CHILD union writes the null parentId)
			final NamespaceId parentId = (NamespaceId) target.getField("parentId");
			if (null == parentId)
				throw new InvalidDescriptorException("child namespace registration requires a parentId");

			parentValue = parentId.value();
		} else {
			parentValue = 0L;
		}

		final byte[] nameBytes = (byte[]) target.getField("name");
		final long rawNamespaceId = IdGenerator.generateNamespaceId(new String(nameBytes, java.nio.charset.StandardCharsets.UTF_8),
				parentValue);
		target.setField("id", new NamespaceId(rawNamespaceId));
	}

	private static void assignMosaicId(final Network network, final CatbufferType target) {
		final byte[] signerBytes = ((org.symbol.sdk.symbol.models.PublicKey) target.getField("signerPublicKey")).bytes();
		final Address address = network.publicKeyToAddress(new PublicKey(signerBytes));
		final MosaicNonce nonce = (MosaicNonce) target.getField("nonce");
		target.setField("id", new MosaicId(IdGenerator.generateMosaicId(address, nonce.value())));
	}

	/**
	 * Deserializes a transaction from a binary payload.
	 *
	 * @param payload Binary payload.
	 * @return Deserialized transaction.
	 */
	public static Transaction deserialize(final byte[] payload) {
		return TransactionFactory.deserialize(payload);
	}

	/**
	 * Deserializes an embedded transaction from a binary payload.
	 *
	 * @param payload Binary payload.
	 * @return Deserialized embedded transaction.
	 */
	public static EmbeddedTransaction deserializeEmbedded(final byte[] payload) {
		return EmbeddedTransactionFactory.deserialize(payload);
	}

	/**
	 * Attaches a signature to a transaction.
	 *
	 * @param transaction Transaction object.
	 * @param signature Signature to attach.
	 * @return JSON transaction payload.
	 */
	public static String attachSignature(final Transaction transaction, final Signature signature) {
		transaction.setField("signature", new org.symbol.sdk.symbol.models.Signature(signature.bytes()));
		return toJson(transaction);
	}

	/**
	 * Generates a JSON representation of transaction that can be sent to a node.
	 *
	 * @param transaction Transaction object.
	 * @return JSON transaction payload.
	 */
	public static String toJson(final Transaction transaction) {
		final String hexPayload = Converter.uint8ToHex(transaction.serialize());
		return "{\"payload\": \"" + hexPayload + "\"}";
	}

	private static RuleBasedTransactionFactory buildRules(final Map<String, Function<Object, Object>> typeRuleOverrides) {
		final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(SymbolTransactionFactory::symbolTypeConverter,
				typeRuleOverrides);
		factory.registerParsers(Models.FACTORIES);

		factory.addStructParser("UnresolvedMosaic", UnresolvedMosaic::new);

		// only the address parsers need overriding (base32, not hex); the generated PublicKey/Hash256/VotingPublicKey parsers already
		// accept
		// hex/bytes/SDK ByteArrays and produce the model type directly, so setField short-circuits instead of rewrapping
		// unresolved fields (e.g. recipientAddress) may hold a namespace alias; the resolved rule produces models.Address directly and
		// rejects an alias, so the field-blind type converter never mis-narrows it
		factory.addPodParser("UnresolvedAddress", Address::parse);
		factory.addPodParser("Address", AddressRules::resolved);

		List.of("UnresolvedMosaicId", "TransactionType", "UnresolvedAddress", "struct:UnresolvedMosaic").forEach(factory::addArrayParser);

		return factory;
	}

	/**
	 * Tries to convert an SDK type to a model type. Returns {@code null} for unrecognized values, which pass through unchanged — the
	 * generated {@code setField} byte-array convert rewraps any remaining SDK ByteArray values.
	 */
	private static Object symbolTypeConverter(final Object value) {
		if (value instanceof Address address)
			return new UnresolvedAddress(address.bytes());

		return null;
	}

}
