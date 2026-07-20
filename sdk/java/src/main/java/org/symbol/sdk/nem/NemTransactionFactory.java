package org.symbol.sdk.nem;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.function.Function;

import org.symbol.sdk.CryptoTypes.Signature;
import org.symbol.sdk.RuleBasedTransactionFactory;
import org.symbol.sdk.nem.models.*;
import org.symbol.sdk.utils.Converter;

/**
 * Factory for creating NEM transactions, wrapping the generated {@link TransactionFactory} with a {@link RuleBasedTransactionFactory}
 * carrying NEM specific rules.
 */
public final class NemTransactionFactory {

	private final RuleBasedTransactionFactory factory;

	private final Network network;

	/**
	 * Creates a factory for the specified network.
	 *
	 * @param network NEM network.
	 */
	public NemTransactionFactory(final Network network) {
		this(network, null);
	}

	/**
	 * Creates a factory for the specified network with per-type rule overrides.
	 *
	 * @param network NEM network.
	 * @param typeRuleOverrides Per-rule parser overrides keyed by rule name. May be {@code null}.
	 */
	public NemTransactionFactory(final Network network, final Map<String, Function<Object, Object>> typeRuleOverrides) {
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
		final Map<String, Object> descriptorWithNetwork = new LinkedHashMap<>(transactionDescriptor);
		descriptorWithNetwork.put("network", Byte.toUnsignedInt(network.identifier));

		final Transaction transaction = (Transaction) factory
				.createFromFactory(entityType -> TransactionFactory.createByName((String) entityType), descriptorWithNetwork);

		if (autosort)
			transaction.sort();

		return transaction;
	}

	/**
	 * Deserializes a transaction from a binary payload.
	 *
	 * @param payload Binary payload.
	 * @return Deserialized transaction.
	 */
	public static Transaction deserialize(final byte[] payload) {
		return deserialize(java.nio.ByteBuffer.wrap(payload));
	}

	/**
	 * Deserializes a transaction from a byte buffer.
	 *
	 * @param buffer Binary payload buffer.
	 * @return Deserialized transaction.
	 */
	public static Transaction deserialize(final java.nio.ByteBuffer buffer) {
		return TransactionFactory.deserialize(buffer);
	}

	/**
	 * Converts a transaction to a non-verifiable transaction; already non-verifiable inputs are returned as-is (idempotent).
	 *
	 * @param transaction Transaction object.
	 * @return Non-verifiable transaction object.
	 */
	public static NonVerifiableTransaction toNonVerifiableTransaction(final Object transaction) {
		if (transaction instanceof Transaction tx)
			return tx.toNonVerifiable();

		if (transaction instanceof NonVerifiableTransaction nv)
			return nv;

		throw new IllegalArgumentException(
				"invalid transaction instance: " + (null == transaction ? "null" : transaction.getClass().getName()));
	}

	/**
	 * Attaches a signature to a transaction.
	 *
	 * @param transaction Transaction object.
	 * @param signature Signature to attach.
	 * @return JSON transaction payload.
	 */
	public static String attachSignature(final Transaction transaction, final Signature signature) {
		transaction.setSignature(new org.symbol.sdk.nem.models.Signature(signature.bytes()));
		return toJson(transaction);
	}

	/**
	 * Generates a JSON representation of transaction that can be sent to a node.
	 *
	 * @param transaction Transaction object.
	 * @return JSON transaction payload.
	 */
	public static String toJson(final Transaction transaction) {
		final String transactionHex = Converter.uint8ToHex(toNonVerifiableTransaction(transaction).serialize());
		final org.symbol.sdk.nem.models.Signature signature = transaction.getSignature();
		final String signatureHex = Converter.uint8ToHex(signature.bytes());
		return "{\"data\":\"" + transactionHex + "\", \"signature\":\"" + signatureHex + "\"}";
	}

	private static RuleBasedTransactionFactory buildRules(final Map<String, Function<Object, Object>> typeRuleOverrides) {
		// NEM has TWO Address types: the hand-written 25-byte raw form and the generated 40-byte
		// base32 wire form — convert to model values before the rules store them
		final Function<Object, Object> nemTypeConverter = value -> {
			if (value instanceof Address handWritten) {
				final byte[] encoded = handWritten.toString().getBytes(java.nio.charset.StandardCharsets.UTF_8);
				return new org.symbol.sdk.nem.models.Address(encoded);
			}

			return null;
		};
		final RuleBasedTransactionFactory factory = new RuleBasedTransactionFactory(nemTypeConverter, typeRuleOverrides);
		factory.addPodParsers(Models.POD_FACTORIES);

		factory.addStructParser("Message", Message::new);
		factory.addStructParser("NamespaceId", NamespaceId::new);
		factory.addStructParser("MosaicId", MosaicId::new);
		factory.addStructParser("Mosaic", Mosaic::new);
		factory.addStructParser("SizePrefixedMosaic", SizePrefixedMosaic::new);
		factory.addStructParser("MosaicLevy", MosaicLevy::new);
		factory.addStructParser("MosaicProperty", MosaicProperty::new);
		factory.addStructParser("SizePrefixedMosaicProperty", SizePrefixedMosaicProperty::new);
		factory.addStructParser("MosaicDefinition", MosaicDefinition::new);
		factory.addStructParser("MultisigAccountModification", MultisigAccountModification::new);
		factory.addStructParser("SizePrefixedMultisigAccountModification", SizePrefixedMultisigAccountModification::new);
		factory.addStructParser("CosignatureV1", CosignatureV1::new);
		factory.addStructParser("SizePrefixedCosignatureV1", SizePrefixedCosignatureV1::new);

		factory.addPodParser("Address", Address::parse);

		List.of("struct:SizePrefixedMosaic", "struct:SizePrefixedMosaicProperty", "struct:SizePrefixedMultisigAccountModification",
				"struct:SizePrefixedCosignatureV1").forEach(factory::addArrayParser);

		return factory;
	}
}
