package org.symbol.sdk.vectors;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.TestFactory;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.Serializer;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.descriptors.AccountAddressRestrictionTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AccountKeyLinkTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AccountMetadataTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AccountMosaicRestrictionTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AccountOperationRestrictionTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AddressAliasTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AggregateBondedTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AggregateBondedTransactionV2Descriptor;
import org.symbol.sdk.symbol.descriptors.AggregateBondedTransactionV3Descriptor;
import org.symbol.sdk.symbol.descriptors.AggregateCompleteTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.AggregateCompleteTransactionV2Descriptor;
import org.symbol.sdk.symbol.descriptors.AggregateCompleteTransactionV3Descriptor;
import org.symbol.sdk.symbol.descriptors.HashLockTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicAddressRestrictionTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicAliasTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicDefinitionTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicGlobalRestrictionTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicMetadataTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicSupplyChangeTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MosaicSupplyRevocationTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.MultisigAccountModificationTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.NamespaceMetadataTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.NamespaceRegistrationTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.NodeKeyLinkTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.SecretLockTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.SecretProofTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.SymbolTransactionDescriptor;
import org.symbol.sdk.symbol.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.UnresolvedMosaicDescriptor;
import org.symbol.sdk.symbol.descriptors.VotingKeyLinkTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.VrfKeyLinkTransactionV1Descriptor;
import org.symbol.sdk.symbol.models.AccountRestrictionFlags;
import org.symbol.sdk.symbol.models.AliasAction;
import org.symbol.sdk.symbol.models.Amount;
import org.symbol.sdk.symbol.models.BlockDuration;
import org.symbol.sdk.symbol.models.Cosignature;
import org.symbol.sdk.symbol.models.EmbeddedTransaction;
import org.symbol.sdk.symbol.models.FinalizationEpoch;
import org.symbol.sdk.symbol.models.LinkAction;
import org.symbol.sdk.symbol.models.LockHashAlgorithm;
import org.symbol.sdk.symbol.models.MosaicFlags;
import org.symbol.sdk.symbol.models.MosaicId;
import org.symbol.sdk.symbol.models.MosaicNonce;
import org.symbol.sdk.symbol.models.MosaicRestrictionType;
import org.symbol.sdk.symbol.models.MosaicSupplyChangeAction;
import org.symbol.sdk.symbol.models.NamespaceId;
import org.symbol.sdk.symbol.models.NamespaceRegistrationType;
import org.symbol.sdk.symbol.models.TransactionType;
import org.symbol.sdk.symbol.models.UnresolvedMosaicId;
import org.symbol.sdk.utils.Converter;

/**
 * Typed-descriptor vector tests for Symbol, tagged {@code catvectors}: every case under
 * {@code tests/vectors/symbol/models/transactions.json} is rebuilt through the generated typed-descriptor API and must serialize
 * byte-for-byte to {@code item.payload}. Port of JS {@code vectors/tsDescriptorsSymbol.js} + the "create from descriptor (typescript)"
 * region of {@code vectors/catbuffer.js}, with two deliberate divergences: the Java aggregate descriptors take pre-built
 * {@code EmbeddedTransaction}/{@code Cosignature} models (nested transaction descriptors were rejected by design), and the header fields
 * typed descriptors deliberately omit (signature, signerPublicKey, fee, deadline) are overlaid on the descriptor map — the analog of the JS
 * rawDescriptor override.
 *
 * Values flow from the vector JSON as-is: strings feed the model constructors ({@code Address},
 * {@code CryptoTypes.PublicKey}/{@code Hash256}) and enum {@code parse} calls, numbers feed the pod {@code parse} coercions (u64 values
 * arrive as wrapped longs — stringifying a negative long would corrupt them), and the byte-carrying fields (message, metadata values,
 * proof) go through {@link CatbufferDescriptorHelper#rawBytes}.
 */
@Tag("catvectors")
final class SymbolTypedDescriptorVectorsTest {

	@TestFactory
	Iterable<DynamicTest> typedDescriptorSymbolTransactions() {
		final SymbolFacade facade = new SymbolFacade("testnet");
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("transactions"),
				item -> assertCreateFromTypedDescriptor(facade, item));
	}

	private static void assertCreateFromTypedDescriptor(final SymbolFacade facade, final Map<String, Object> item) {
		// Arrange:
		final Map<String, Object> plain = CatbufferDescriptorHelper.normalizeInput((Map<?, ?>) item.get("descriptor"));
		final Map<String, Object> descriptor = toFactoryDescriptor(plain, facade);

		// Act:
		final Serializer transaction = facade.transactionFactory.create(descriptor);

		// Assert:
		CatbufferVectorsHelper.assertPayload(item, transaction, "typed-descriptor");
	}

	// region typed descriptor mapping

	/**
	 * Builds the factory descriptor map from the typed descriptor, overlaying the header fields typed descriptors deliberately omit
	 * (mirrors the JS tsDescriptorsSymbol default export).
	 */
	private static Map<String, Object> toFactoryDescriptor(final Map<String, Object> plain, final SymbolFacade facade) {
		final Map<String, Object> descriptor = new LinkedHashMap<>(createTypedTransactionDescriptor(plain, facade).toMap());
		descriptor.put("signature", plain.get("signature"));
		descriptor.put("signerPublicKey", plain.get("signerPublicKey"));
		descriptor.put("fee", plain.get("fee"));
		descriptor.put("deadline", plain.get("deadline"));
		return descriptor;
	}

	@FunctionalInterface
	private interface AggregateDescriptorFactory {
		SymbolTransactionDescriptor create(CryptoTypes.Hash256 transactionsHash, List<EmbeddedTransaction> transactions,
				List<Cosignature> cosignatures);
	}

	// the six aggregate variants differ only in the constructed type; the shared tail lives once in createTypedTransactionDescriptor
	private static final Map<String, AggregateDescriptorFactory> AGGREGATE_FACTORIES = Map.of("aggregate_bonded_transaction_v1",
			AggregateBondedTransactionV1Descriptor::new, "aggregate_bonded_transaction_v2", AggregateBondedTransactionV2Descriptor::new,
			"aggregate_bonded_transaction_v3", AggregateBondedTransactionV3Descriptor::new, "aggregate_complete_transaction_v1",
			AggregateCompleteTransactionV1Descriptor::new, "aggregate_complete_transaction_v2",
			AggregateCompleteTransactionV2Descriptor::new, "aggregate_complete_transaction_v3",
			AggregateCompleteTransactionV3Descriptor::new);

	private static SymbolTransactionDescriptor createTypedTransactionDescriptor(final Map<String, Object> plain,
			final SymbolFacade facade) {
		final String type = (String) plain.get("type");
		final AggregateDescriptorFactory aggregateFactory = AGGREGATE_FACTORIES.get(type);
		if (null != aggregateFactory)
			return aggregateFactory.create(getTransactionsHash(plain), mapEmbedded(plain, facade), mapCosignatures(plain));

		switch (type) {
			case "account_key_link_transaction_v1":
				return new AccountKeyLinkTransactionV1Descriptor(new CryptoTypes.PublicKey((String) plain.get("linkedPublicKey")),
						LinkAction.parse(plain.get("linkAction")));

			case "node_key_link_transaction_v1":
				return new NodeKeyLinkTransactionV1Descriptor(new CryptoTypes.PublicKey((String) plain.get("linkedPublicKey")),
						LinkAction.parse(plain.get("linkAction")));

			case "voting_key_link_transaction_v1":
				return new VotingKeyLinkTransactionV1Descriptor(new CryptoTypes.PublicKey((String) plain.get("linkedPublicKey")),
						FinalizationEpoch.parse(plain.get("startEpoch")), FinalizationEpoch.parse(plain.get("endEpoch")),
						LinkAction.parse(plain.get("linkAction")));

			case "vrf_key_link_transaction_v1":
				return new VrfKeyLinkTransactionV1Descriptor(new CryptoTypes.PublicKey((String) plain.get("linkedPublicKey")),
						LinkAction.parse(plain.get("linkAction")));

			case "hash_lock_transaction_v1":
				return new HashLockTransactionV1Descriptor(mapUnresolvedMosaic(CatbufferVectorsHelper.toObjectMap(plain.get("mosaic"))),
						BlockDuration.parse(plain.get("duration")), new CryptoTypes.Hash256((String) plain.get("hash")));

			case "secret_lock_transaction_v1":
				return new SecretLockTransactionV1Descriptor(new Address((String) plain.get("recipientAddress")),
						new CryptoTypes.Hash256((String) plain.get("secret")),
						mapUnresolvedMosaic(CatbufferVectorsHelper.toObjectMap(plain.get("mosaic"))),
						BlockDuration.parse(plain.get("duration")), LockHashAlgorithm.parse(plain.get("hashAlgorithm")));

			case "secret_proof_transaction_v1":
				return new SecretProofTransactionV1Descriptor(new Address((String) plain.get("recipientAddress")),
						new CryptoTypes.Hash256((String) plain.get("secret")), LockHashAlgorithm.parse(plain.get("hashAlgorithm")),
						CatbufferDescriptorHelper.rawBytes(plain.get("proof")));

			case "account_metadata_transaction_v1":
				return new AccountMetadataTransactionV1Descriptor(new Address((String) plain.get("targetAddress")),
						Converter.toLong((Number) plain.get("scopedMetadataKey")), Converter.toInt((Number) plain.get("valueSizeDelta")),
						CatbufferDescriptorHelper.rawBytes(plain.get("value")));

			case "mosaic_metadata_transaction_v1":
				return new MosaicMetadataTransactionV1Descriptor(new Address((String) plain.get("targetAddress")),
						Converter.toLong((Number) plain.get("scopedMetadataKey")), UnresolvedMosaicId.parse(plain.get("targetMosaicId")),
						Converter.toInt((Number) plain.get("valueSizeDelta")), CatbufferDescriptorHelper.rawBytes(plain.get("value")));

			case "namespace_metadata_transaction_v1":
				// deliberately plain text even when it looks like hex — see CatbufferDescriptorHelper.isPlainTextValue
				return new NamespaceMetadataTransactionV1Descriptor(new Address((String) plain.get("targetAddress")),
						Converter.toLong((Number) plain.get("scopedMetadataKey")), NamespaceId.parse(plain.get("targetNamespaceId")),
						Converter.toInt((Number) plain.get("valueSizeDelta")),
						((String) plain.get("value")).getBytes(StandardCharsets.UTF_8));

			case "mosaic_definition_transaction_v1":
				// placeholder id mirrors JS: the factory autogenerates the mosaic id from nonce + signer
				return new MosaicDefinitionTransactionV1Descriptor(new MosaicId(0L), BlockDuration.parse(plain.get("duration")),
						MosaicNonce.parse(plain.get("nonce")), MosaicFlags.parse(plain.get("flags")),
						Converter.toInt((Number) plain.get("divisibility")));

			case "mosaic_supply_change_transaction_v1":
				return new MosaicSupplyChangeTransactionV1Descriptor(UnresolvedMosaicId.parse(plain.get("mosaicId")),
						Amount.parse(plain.get("delta")), MosaicSupplyChangeAction.parse(plain.get("action")));

			case "mosaic_supply_revocation_transaction_v1":
				return new MosaicSupplyRevocationTransactionV1Descriptor(new Address((String) plain.get("sourceAddress")),
						mapUnresolvedMosaic(CatbufferVectorsHelper.toObjectMap(plain.get("mosaic"))));

			case "multisig_account_modification_transaction_v1":
				return new MultisigAccountModificationTransactionV1Descriptor(Converter.toInt((Number) plain.get("minRemovalDelta")),
						Converter.toInt((Number) plain.get("minApprovalDelta")), mapAddressList(plain.get("addressAdditions")),
						mapAddressList(plain.get("addressDeletions")));

			case "address_alias_transaction_v1":
				return new AddressAliasTransactionV1Descriptor(NamespaceId.parse(plain.get("namespaceId")),
						new Address((String) plain.get("address")), AliasAction.parse(plain.get("aliasAction")));

			case "mosaic_alias_transaction_v1":
				return new MosaicAliasTransactionV1Descriptor(NamespaceId.parse(plain.get("namespaceId")),
						MosaicId.parse(plain.get("mosaicId")), AliasAction.parse(plain.get("aliasAction")));

			case "namespace_registration_transaction_v1":
				// placeholder id mirrors JS: the factory autogenerates the namespace id from the name;
				// duration is only present for 'root' registrations and parentId only for 'child'
				return new NamespaceRegistrationTransactionV1Descriptor(new NamespaceId(0L),
						NamespaceRegistrationType.parse(plain.get("registrationType")),
						null == plain.get("duration") ? null : BlockDuration.parse(plain.get("duration")),
						null == plain.get("parentId") ? null : NamespaceId.parse(plain.get("parentId")),
						((String) plain.get("name")).getBytes(StandardCharsets.UTF_8));

			case "account_address_restriction_transaction_v1":
				return new AccountAddressRestrictionTransactionV1Descriptor(AccountRestrictionFlags.parse(plain.get("restrictionFlags")),
						mapAddressList(plain.get("restrictionAdditions")), mapAddressList(plain.get("restrictionDeletions")));

			case "account_mosaic_restriction_transaction_v1":
				return new AccountMosaicRestrictionTransactionV1Descriptor(AccountRestrictionFlags.parse(plain.get("restrictionFlags")),
						mapMosaicIdList(plain.get("restrictionAdditions")), mapMosaicIdList(plain.get("restrictionDeletions")));

			case "account_operation_restriction_transaction_v1":
				return new AccountOperationRestrictionTransactionV1Descriptor(AccountRestrictionFlags.parse(plain.get("restrictionFlags")),
						mapTransactionTypeList(plain.get("restrictionAdditions")),
						mapTransactionTypeList(plain.get("restrictionDeletions")));

			case "mosaic_address_restriction_transaction_v1":
				return new MosaicAddressRestrictionTransactionV1Descriptor(UnresolvedMosaicId.parse(plain.get("mosaicId")),
						Converter.toLong((Number) plain.get("restrictionKey")),
						Converter.toLong((Number) plain.get("previousRestrictionValue")),
						Converter.toLong((Number) plain.get("newRestrictionValue")), new Address((String) plain.get("targetAddress")));

			case "mosaic_global_restriction_transaction_v1":
				return new MosaicGlobalRestrictionTransactionV1Descriptor(UnresolvedMosaicId.parse(plain.get("mosaicId")),
						UnresolvedMosaicId.parse(plain.get("referenceMosaicId")), Converter.toLong((Number) plain.get("restrictionKey")),
						Converter.toLong((Number) plain.get("previousRestrictionValue")),
						Converter.toLong((Number) plain.get("newRestrictionValue")),
						MosaicRestrictionType.parse(plain.get("previousRestrictionType")),
						MosaicRestrictionType.parse(plain.get("newRestrictionType")));

			case "transfer_transaction_v1":
				return new TransferTransactionV1Descriptor(new Address((String) plain.get("recipientAddress")),
						mapUnresolvedMosaicList(plain.get("mosaics")),
						null == plain.get("message") ? null : CatbufferDescriptorHelper.rawBytes(plain.get("message")));

			default :
				throw new IllegalArgumentException("unknown transaction type " + type);
		}
	}

	// endregion

	// region subobject mapping

	private static UnresolvedMosaicDescriptor mapUnresolvedMosaic(final Map<String, Object> mosaic) {
		return new UnresolvedMosaicDescriptor(UnresolvedMosaicId.parse(mosaic.get("mosaicId")), Amount.parse(mosaic.get("amount")));
	}

	// the list mappers pass an absent vector field through as null, which the all-args constructors omit from the map

	private static List<UnresolvedMosaicDescriptor> mapUnresolvedMosaicList(final Object mosaics) {
		if (null == mosaics)
			return null;

		final List<UnresolvedMosaicDescriptor> result = new ArrayList<>();
		for (final Object element : (List<?>) mosaics)
			result.add(mapUnresolvedMosaic(CatbufferVectorsHelper.toObjectMap(element)));

		return result;
	}

	private static List<Address> mapAddressList(final Object addresses) {
		if (null == addresses)
			return null;

		final List<Address> result = new ArrayList<>();
		for (final Object element : (List<?>) addresses)
			result.add(new Address((String) element));

		return result;
	}

	private static List<TransactionType> mapTransactionTypeList(final Object types) {
		if (null == types)
			return null;

		final List<TransactionType> result = new ArrayList<>();
		for (final Object element : (List<?>) types)
			result.add(TransactionType.parse(element));

		return result;
	}

	private static List<UnresolvedMosaicId> mapMosaicIdList(final Object mosaicIds) {
		if (null == mosaicIds)
			return null;

		final List<UnresolvedMosaicId> result = new ArrayList<>();
		for (final Object element : (List<?>) mosaicIds)
			result.add(UnresolvedMosaicId.parse(element));

		return result;
	}

	/**
	 * Maps aggregate children through the typed-descriptor mapper and the embedded factory; nested transaction descriptors were
	 * deliberately rejected in the Java API, so the aggregate descriptor takes the pre-built models.
	 */
	private static List<EmbeddedTransaction> mapEmbedded(final Map<String, Object> plain, final SymbolFacade facade) {
		final List<EmbeddedTransaction> embedded = new ArrayList<>();
		if (null == plain.get("transactions"))
			return embedded;

		for (final Object element : (List<?>) plain.get("transactions")) {
			final Map<String, Object> child = CatbufferVectorsHelper.toObjectMap(element);
			embedded.add(facade.transactionFactory.createEmbedded(createTypedTransactionDescriptor(child, facade).toMap()));
		}

		return embedded;
	}

	private static List<Cosignature> mapCosignatures(final Map<String, Object> plain) {
		final List<Cosignature> cosignatures = new ArrayList<>();
		if (null == plain.get("cosignatures"))
			return cosignatures;

		for (final Object element : (List<?>) plain.get("cosignatures"))
			cosignatures.add(CatbufferVectorsHelper.toSymbolCosignature(CatbufferVectorsHelper.toObjectMap(element)));

		return cosignatures;
	}

	// endregion

	// region value helpers

	private static CryptoTypes.Hash256 getTransactionsHash(final Map<String, Object> plain) {
		return new CryptoTypes.Hash256((String) plain.get("transactionsHash"));
	}

	// endregion
}
