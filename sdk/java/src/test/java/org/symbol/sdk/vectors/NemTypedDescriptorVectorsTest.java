package org.symbol.sdk.vectors;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.TestFactory;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.Serializer;
import org.symbol.sdk.facade.NemFacade;
import org.symbol.sdk.nem.Address;
import org.symbol.sdk.nem.NemTransactionFactory;
import org.symbol.sdk.nem.descriptors.AccountKeyLinkTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.CosignatureV1Descriptor;
import org.symbol.sdk.nem.descriptors.MessageDescriptor;
import org.symbol.sdk.nem.descriptors.MosaicDefinitionDescriptor;
import org.symbol.sdk.nem.descriptors.MosaicDefinitionTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.MosaicDescriptor;
import org.symbol.sdk.nem.descriptors.MosaicIdDescriptor;
import org.symbol.sdk.nem.descriptors.MosaicLevyDescriptor;
import org.symbol.sdk.nem.descriptors.MosaicPropertyDescriptor;
import org.symbol.sdk.nem.descriptors.MosaicSupplyChangeTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.MultisigAccountModificationDescriptor;
import org.symbol.sdk.nem.descriptors.MultisigAccountModificationTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.MultisigAccountModificationTransactionV2Descriptor;
import org.symbol.sdk.nem.descriptors.MultisigTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.NamespaceIdDescriptor;
import org.symbol.sdk.nem.descriptors.NamespaceRegistrationTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.NemTransactionDescriptor;
import org.symbol.sdk.nem.descriptors.SizePrefixedMosaicDescriptor;
import org.symbol.sdk.nem.descriptors.SizePrefixedMosaicPropertyDescriptor;
import org.symbol.sdk.nem.descriptors.SizePrefixedMultisigAccountModificationDescriptor;
import org.symbol.sdk.nem.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.nem.descriptors.TransferTransactionV2Descriptor;
import org.symbol.sdk.nem.models.Amount;
import org.symbol.sdk.nem.models.LinkAction;
import org.symbol.sdk.nem.models.MessageType;
import org.symbol.sdk.nem.models.MosaicSupplyChangeAction;
import org.symbol.sdk.nem.models.MosaicTransferFeeType;
import org.symbol.sdk.nem.models.MultisigAccountModificationType;
import org.symbol.sdk.nem.models.NonVerifiableTransaction;
import org.symbol.sdk.nem.models.SizePrefixedCosignatureV1;
import org.symbol.sdk.utils.Converter;

/**
 * Typed-descriptor vector tests for NEM, tagged {@code catvectors}: every case under {@code tests/vectors/nem/models/transactions.json} is
 * rebuilt through the generated typed-descriptor API and must serialize byte-for-byte to {@code item.payload}. Port of JS
 * {@code vectors/tsDescriptorsNem.js} + the "create from descriptor (typescript)" region of {@code vectors/catbuffer.js}, with two
 * deliberate divergences: the Java multisig descriptor takes a pre-built {@code NonVerifiableTransaction} (nested transaction descriptors
 * were rejected by design), and the header fields typed descriptors deliberately omit (signature, timestamp, signerPublicKey, fee) are
 * overlaid on the descriptor map — the analog of the JS rawDescriptor override.
 *
 * Values flow from the vector JSON as-is: strings feed the model constructors ({@code Address},
 * {@code CryptoTypes.PublicKey}/{@code Hash256}) and enum {@code parse} calls, numbers feed the pod {@code parse} coercions (u64 values
 * arrive as wrapped longs — stringifying a negative long would corrupt them), and the byte-carrying fields (message content, names,
 * descriptions, property name/value pairs — hex-encoded in the NEM vectors) go through {@link CatbufferDescriptorHelper#rawBytes}.
 */
@Tag("catvectors")
final class NemTypedDescriptorVectorsTest {

	@TestFactory
	Iterable<DynamicTest> typedDescriptorNemTransactions() {
		final NemFacade facade = new NemFacade("testnet");
		return CatbufferVectorsHelper.perCaseTests("nem", List.of("transactions"), item -> assertCreateFromTypedDescriptor(facade, item));
	}

	private static void assertCreateFromTypedDescriptor(final NemFacade facade, final Map<String, Object> item) {
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
	 * (mirrors the JS tsDescriptorsNem default export), then wraps multisig cosignatures at map level.
	 */
	private static Map<String, Object> toFactoryDescriptor(final Map<String, Object> plain, final NemFacade facade) {
		final Map<String, Object> descriptor = new LinkedHashMap<>(createTypedTransactionDescriptor(plain, facade).toMap());
		descriptor.put("signature", plain.get("signature"));
		descriptor.put("timestamp", plain.get("timestamp"));
		descriptor.put("signerPublicKey", plain.get("signerPublicKey"));
		descriptor.put("fee", plain.get("fee"));

		if (plain.containsKey("cosignatures"))
			descriptor.put("cosignatures", mapCosignatures(plain, facade));

		return descriptor;
	}

	private static NemTransactionDescriptor createTypedTransactionDescriptor(final Map<String, Object> plain, final NemFacade facade) {
		final String type = (String) plain.get("type");
		switch (type) {
			case "account_key_link_transaction_v1":
				return new AccountKeyLinkTransactionV1Descriptor(LinkAction.parse(plain.get("linkAction")),
						new CryptoTypes.PublicKey((String) plain.get("remotePublicKey")));

			case "mosaic_definition_transaction_v1":
				return new MosaicDefinitionTransactionV1Descriptor(
						mapMosaicDefinition(CatbufferVectorsHelper.toObjectMap(plain.get("mosaicDefinition"))),
						new Address((String) plain.get("rentalFeeSink")), Amount.parse(plain.get("rentalFee")));

			case "mosaic_supply_change_transaction_v1":
				return new MosaicSupplyChangeTransactionV1Descriptor(mapMosaicId(CatbufferVectorsHelper.toObjectMap(plain.get("mosaicId"))),
						MosaicSupplyChangeAction.parse(plain.get("action")), Amount.parse(plain.get("delta")));

			case "multisig_account_modification_transaction_v1":
				return new MultisigAccountModificationTransactionV1Descriptor(mapModifications(plain));

			case "multisig_account_modification_transaction_v2":
				return new MultisigAccountModificationTransactionV2Descriptor(Converter.toInt((Number) plain.get("minApprovalDelta")),
						mapModifications(plain));

			case "namespace_registration_transaction_v1":
				return new NamespaceRegistrationTransactionV1Descriptor(new Address((String) plain.get("rentalFeeSink")),
						Amount.parse(plain.get("rentalFee")), CatbufferDescriptorHelper.rawBytes(plain.get("name")),
						null == plain.get("parentName") ? null : CatbufferDescriptorHelper.rawBytes(plain.get("parentName")));

			case "transfer_transaction_v1":
				return new TransferTransactionV1Descriptor(new Address((String) plain.get("recipientAddress")),
						Amount.parse(plain.get("amount")), mapOptionalMessage(plain));

			case "transfer_transaction_v2":
				return new TransferTransactionV2Descriptor(new Address((String) plain.get("recipientAddress")),
						Amount.parse(plain.get("amount")), mapOptionalMessage(plain), mapMosaicList(plain.get("mosaics")));

			case "cosignature_v1":
				return new CosignatureV1Descriptor(new CryptoTypes.Hash256((String) plain.get("otherTransactionHash")),
						new Address((String) plain.get("multisigAccountAddress")));

			case "multisig_transaction_v1": {
				final Map<String, Object> innerPlain = CatbufferVectorsHelper.toObjectMap(plain.get("innerTransaction"));
				final Map<String, Object> innerDescriptor = new LinkedHashMap<>(
						createTypedTransactionDescriptor(innerPlain, facade).toMap());
				// set the inner transaction's own base header (its typed descriptor omits these facade-owned
				// fields); mirrors how Symbol's mapEmbedded builds each child from its own data
				innerDescriptor.put("timestamp", innerPlain.get("timestamp"));
				innerDescriptor.put("signerPublicKey", innerPlain.get("signerPublicKey"));
				innerDescriptor.put("fee", innerPlain.get("fee"));

				final NonVerifiableTransaction inner = NemTransactionFactory
						.toNonVerifiableTransaction(facade.transactionFactory.create(innerDescriptor));
				return new MultisigTransactionV1Descriptor(inner, /* cosignatures */ null);
			}

			default :
				throw new IllegalArgumentException("unknown transaction type " + type);
		}
	}

	private static List<SizePrefixedCosignatureV1> mapCosignatures(final Map<String, Object> plain, final NemFacade facade) {
		final List<SizePrefixedCosignatureV1> cosignatures = new ArrayList<>();
		for (final Object element : (List<?>) plain.get("cosignatures")) {
			final Map<String, Object> cosignature = extractNamedChild(element, "cosignature");
			final CosignatureV1Descriptor typedCosignature = new CosignatureV1Descriptor(
					new CryptoTypes.Hash256((String) cosignature.get("otherTransactionHash")),
					new Address((String) cosignature.get("multisigAccountAddress")));

			final Map<String, Object> cosignatureDescriptor = new LinkedHashMap<>(typedCosignature.toMap());
			// override base transaction properties to get vectors to pass (the analog of the JS rawDescriptor override)
			cosignatureDescriptor.put("signature", cosignature.get("signature"));
			cosignatureDescriptor.put("timestamp", cosignature.get("timestamp"));
			cosignatureDescriptor.put("signerPublicKey", cosignature.get("signerPublicKey"));
			cosignatureDescriptor.put("fee", cosignature.get("fee"));

			cosignatures.add(CatbufferVectorsHelper.wrapNemCosignature(facade, cosignatureDescriptor));
		}

		return cosignatures;
	}

	// the optional-field mappers pass an absent vector field through as null, which the all-args constructors omit from the map

	private static MessageDescriptor mapOptionalMessage(final Map<String, Object> plain) {
		if (null == plain.get("message"))
			return null;

		final Map<String, Object> message = CatbufferVectorsHelper.toObjectMap(plain.get("message"));
		return new MessageDescriptor(MessageType.parse(message.get("messageType")),
				CatbufferDescriptorHelper.rawBytes(message.get("message")));
	}

	private static List<SizePrefixedMosaicDescriptor> mapMosaicList(final Object mosaics) {
		if (null == mosaics)
			return null;

		final List<SizePrefixedMosaicDescriptor> result = new ArrayList<>();
		for (final Object element : (List<?>) mosaics) {
			final Map<String, Object> mosaic = extractNamedChild(element, "mosaic");
			result.add(new SizePrefixedMosaicDescriptor(new MosaicDescriptor(
					mapMosaicId(CatbufferVectorsHelper.toObjectMap(mosaic.get("mosaicId"))), Amount.parse(mosaic.get("amount")))));
		}

		return result;
	}

	private static MosaicIdDescriptor mapMosaicId(final Map<String, Object> mosaicId) {
		final Map<String, Object> namespaceId = CatbufferVectorsHelper.toObjectMap(mosaicId.get("namespaceId"));
		return new MosaicIdDescriptor(new NamespaceIdDescriptor(CatbufferDescriptorHelper.rawBytes(namespaceId.get("name"))),
				CatbufferDescriptorHelper.rawBytes(mosaicId.get("name")));
	}

	private static MosaicDefinitionDescriptor mapMosaicDefinition(final Map<String, Object> definition) {
		return new MosaicDefinitionDescriptor(new CryptoTypes.PublicKey((String) definition.get("ownerPublicKey")),
				mapMosaicId(CatbufferVectorsHelper.toObjectMap(definition.get("id"))),
				CatbufferDescriptorHelper.rawBytes(definition.get("description")), mapProperties(definition.get("properties")),
				mapLevy(definition.get("levy")));
	}

	private static List<SizePrefixedMosaicPropertyDescriptor> mapProperties(final Object propertyList) {
		if (null == propertyList)
			return null;

		final List<SizePrefixedMosaicPropertyDescriptor> properties = new ArrayList<>();
		for (final Object element : (List<?>) propertyList) {
			final Map<String, Object> property = extractNamedChild(element, "property");
			properties.add(new SizePrefixedMosaicPropertyDescriptor(new MosaicPropertyDescriptor(
					CatbufferDescriptorHelper.rawBytes(property.get("name")), CatbufferDescriptorHelper.rawBytes(property.get("value")))));
		}

		return properties;
	}

	private static MosaicLevyDescriptor mapLevy(final Object levyValue) {
		if (null == levyValue)
			return null;

		final Map<String, Object> levy = CatbufferVectorsHelper.toObjectMap(levyValue);
		return new MosaicLevyDescriptor(MosaicTransferFeeType.parse(levy.get("transferFeeType")),
				new Address((String) levy.get("recipientAddress")), mapMosaicId(CatbufferVectorsHelper.toObjectMap(levy.get("mosaicId"))),
				Amount.parse(levy.get("fee")));
	}

	private static List<SizePrefixedMultisigAccountModificationDescriptor> mapModifications(final Map<String, Object> plain) {
		final List<SizePrefixedMultisigAccountModificationDescriptor> modifications = new ArrayList<>();
		for (final Object element : (List<?>) plain.get("modifications")) {
			final Map<String, Object> modification = extractNamedChild(element, "modification");
			modifications.add(new SizePrefixedMultisigAccountModificationDescriptor(
					new MultisigAccountModificationDescriptor(MultisigAccountModificationType.parse(modification.get("modificationType")),
							new CryptoTypes.PublicKey((String) modification.get("cosignatoryPublicKey")))));
		}

		return modifications;
	}

	private static Map<String, Object> extractNamedChild(final Object container, final String childName) {
		return CatbufferVectorsHelper.toObjectMap(CatbufferVectorsHelper.toObjectMap(container).get(childName));
	}

	// endregion

}
