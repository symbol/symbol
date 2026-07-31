package org.symbol.sdk.vectors;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.TestFactory;

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
import org.symbol.sdk.nem.models.MosaicSupplyChangeAction;
import org.symbol.sdk.nem.models.MosaicTransferFeeType;
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
 * Values flow from the vector JSON as-is: strings feed the String constructor overloads, numbers feed the pod {@code parse} coercions (u64
 * values arrive as wrapped longs — stringifying a negative long would corrupt them), and the byte-carrying fields (message content, names,
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
				return new AccountKeyLinkTransactionV1Descriptor((String) plain.get("linkAction"), (String) plain.get("remotePublicKey"));

			case "mosaic_definition_transaction_v1":
				return new MosaicDefinitionTransactionV1Descriptor(
						mapMosaicDefinition(CatbufferVectorsHelper.toObjectMap(plain.get("mosaicDefinition"))),
						new Address((String) plain.get("rentalFeeSink")), Amount.parse(plain.get("rentalFee")));

			case "mosaic_supply_change_transaction_v1":
				return new MosaicSupplyChangeTransactionV1Descriptor(mapMosaicId(CatbufferVectorsHelper.toObjectMap(plain.get("mosaicId"))),
						MosaicSupplyChangeAction.parse(plain.get("action")), Amount.parse(plain.get("delta")));

			case "multisig_account_modification_transaction_v1":
				return new MultisigAccountModificationTransactionV1Descriptor().modifications(mapModifications(plain));

			case "multisig_account_modification_transaction_v2":
				return new MultisigAccountModificationTransactionV2Descriptor(Converter.toInt((Number) plain.get("minApprovalDelta")))
						.modifications(mapModifications(plain));

			case "namespace_registration_transaction_v1": {
				final NamespaceRegistrationTransactionV1Descriptor descriptor = new NamespaceRegistrationTransactionV1Descriptor(
						new Address((String) plain.get("rentalFeeSink")), Amount.parse(plain.get("rentalFee")))
						.name(CatbufferDescriptorHelper.rawBytes(plain.get("name")));
				if (null != plain.get("parentName"))
					descriptor.parentName(CatbufferDescriptorHelper.rawBytes(plain.get("parentName")));

				return descriptor;
			}

			case "transfer_transaction_v1": {
				final TransferTransactionV1Descriptor descriptor = new TransferTransactionV1Descriptor(
						new Address((String) plain.get("recipientAddress")), Amount.parse(plain.get("amount")));
				if (null != plain.get("message"))
					descriptor.message(mapMessage(CatbufferVectorsHelper.toObjectMap(plain.get("message"))));

				return descriptor;
			}

			case "transfer_transaction_v2": {
				final TransferTransactionV2Descriptor descriptor = new TransferTransactionV2Descriptor(
						new Address((String) plain.get("recipientAddress")), Amount.parse(plain.get("amount")));
				if (null != plain.get("message"))
					descriptor.message(mapMessage(CatbufferVectorsHelper.toObjectMap(plain.get("message"))));

				if (null != plain.get("mosaics")) {
					final List<SizePrefixedMosaicDescriptor> mosaics = new ArrayList<>();
					for (final Object element : (List<?>) plain.get("mosaics")) {
						final Map<String, Object> mosaic = extractNamedChild(element, "mosaic");
						mosaics.add(new SizePrefixedMosaicDescriptor(
								new MosaicDescriptor(mapMosaicId(CatbufferVectorsHelper.toObjectMap(mosaic.get("mosaicId"))),
										Amount.parse(mosaic.get("amount")))));
					}

					descriptor.mosaics(mosaics);
				}

				return descriptor;
			}

			case "cosignature_v1":
				return new CosignatureV1Descriptor((String) plain.get("otherTransactionHash"),
						(String) plain.get("multisigAccountAddress"));

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
				return new MultisigTransactionV1Descriptor(inner);
			}

			default :
				throw new IllegalArgumentException("unknown transaction type " + type);
		}
	}

	private static List<SizePrefixedCosignatureV1> mapCosignatures(final Map<String, Object> plain, final NemFacade facade) {
		final List<SizePrefixedCosignatureV1> cosignatures = new ArrayList<>();
		for (final Object element : (List<?>) plain.get("cosignatures")) {
			final Map<String, Object> cosignature = extractNamedChild(element, "cosignature");
			final CosignatureV1Descriptor typedCosignature = new CosignatureV1Descriptor((String) cosignature.get("otherTransactionHash"),
					(String) cosignature.get("multisigAccountAddress"));

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

	private static MessageDescriptor mapMessage(final Map<String, Object> message) {
		return new MessageDescriptor((String) message.get("messageType"))
				.message(CatbufferDescriptorHelper.rawBytes(message.get("message")));
	}

	private static MosaicIdDescriptor mapMosaicId(final Map<String, Object> mosaicId) {
		final Map<String, Object> namespaceId = CatbufferVectorsHelper.toObjectMap(mosaicId.get("namespaceId"));
		return new MosaicIdDescriptor(new NamespaceIdDescriptor().name(CatbufferDescriptorHelper.rawBytes(namespaceId.get("name"))))
				.name(CatbufferDescriptorHelper.rawBytes(mosaicId.get("name")));
	}

	private static MosaicDefinitionDescriptor mapMosaicDefinition(final Map<String, Object> definition) {
		final MosaicDefinitionDescriptor descriptor = new MosaicDefinitionDescriptor((String) definition.get("ownerPublicKey"),
				mapMosaicId(CatbufferVectorsHelper.toObjectMap(definition.get("id"))))
				.description(CatbufferDescriptorHelper.rawBytes(definition.get("description")));

		if (null != definition.get("properties")) {
			final List<SizePrefixedMosaicPropertyDescriptor> properties = new ArrayList<>();
			for (final Object element : (List<?>) definition.get("properties")) {
				final Map<String, Object> property = extractNamedChild(element, "property");
				properties.add(new SizePrefixedMosaicPropertyDescriptor(
						new MosaicPropertyDescriptor().name(CatbufferDescriptorHelper.rawBytes(property.get("name")))
								.value(CatbufferDescriptorHelper.rawBytes(property.get("value")))));
			}

			descriptor.properties(properties);
		}

		if (null != definition.get("levy")) {
			final Map<String, Object> levy = CatbufferVectorsHelper.toObjectMap(definition.get("levy"));
			descriptor.levy(new MosaicLevyDescriptor(MosaicTransferFeeType.parse(levy.get("transferFeeType")),
					new Address((String) levy.get("recipientAddress")),
					mapMosaicId(CatbufferVectorsHelper.toObjectMap(levy.get("mosaicId"))), Amount.parse(levy.get("fee"))));
		}

		return descriptor;
	}

	private static List<SizePrefixedMultisigAccountModificationDescriptor> mapModifications(final Map<String, Object> plain) {
		final List<SizePrefixedMultisigAccountModificationDescriptor> modifications = new ArrayList<>();
		for (final Object element : (List<?>) plain.get("modifications")) {
			final Map<String, Object> modification = extractNamedChild(element, "modification");
			modifications.add(new SizePrefixedMultisigAccountModificationDescriptor(new MultisigAccountModificationDescriptor(
					(String) modification.get("modificationType"), (String) modification.get("cosignatoryPublicKey"))));
		}

		return modifications;
	}

	private static Map<String, Object> extractNamedChild(final Object container, final String childName) {
		return CatbufferVectorsHelper.toObjectMap(CatbufferVectorsHelper.toObjectMap(container).get(childName));
	}

	// endregion

}
