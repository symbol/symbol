package org.symbol.sdk.vectors;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.hasKey;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.function.Function;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.TestFactory;

import org.symbol.sdk.Serializer;
import org.symbol.sdk.facade.NemFacade;
import org.symbol.sdk.facade.SymbolFacade;

/**
 * Descriptor-driven catbuffer vector tests, tagged {@code catvectors} ({@code ./gradlew catVectors}; requires {@code SCHEMAS_PATH} pointing
 * at {@code tests/vectors}). Each case under {@code tests/vectors/{nem,symbol}/models/*.json} is normalized, dispatched to the factory
 * matching the source filename (transactions / blocks / receipts), and must serialize back to {@code item.payload}.
 */
@Tag("catvectors")
final class CatbufferDescriptorVectorsTest {
	// region NEM transactions

	@TestFactory
	Iterable<DynamicTest> descriptorNemTransactions() {
		final NemFacade facade = new NemFacade("testnet");
		return CatbufferVectorsHelper.perCaseTests("nem", List.of("transactions"),
				item -> assertCreateNemTransactionFromDescriptor(facade, item));
	}

	private static void assertCreateNemTransactionFromDescriptor(final NemFacade facade, final Map<String, Object> item) {
		final Map<String, Object> descriptor = CatbufferDescriptorHelper.normalizeInput((Map<?, ?>) item.get("descriptor"));
		CatbufferDescriptorHelper.fixupDescriptorCommon(descriptor);
		fixupNemAggregate(facade, descriptor);

		final Serializer transaction = facade.transactionFactory.create(descriptor);
		assertSchemaType(item, transaction);
		CatbufferVectorsHelper.assertPayload(item, transaction, "descriptor");
		assertConversions(descriptor, transaction);
	}

	private static void fixupNemAggregate(final NemFacade facade, final Map<String, Object> descriptor) {
		final Object inner = descriptor.get("innerTransaction");
		if (!(inner instanceof Map<?, ?> innerMap))
			return;

		final Map<String, Object> innerDescriptor = CatbufferVectorsHelper.toObjectMap(innerMap);
		descriptor.put("innerTransaction",
				org.symbol.sdk.nem.NemTransactionFactory.toNonVerifiableTransaction(facade.transactionFactory.create(innerDescriptor)));

		final Object cosignatures = descriptor.get("cosignatures");
		if (!(cosignatures instanceof List<?> list))
			return;

		final List<org.symbol.sdk.nem.models.SizePrefixedCosignatureV1> wrapped = new ArrayList<>();
		for (final Object element : list) {
			final Map<?, ?> cosignatureContainer = (Map<?, ?>) element;
			final Map<String, Object> cosignatureDescriptor = CatbufferVectorsHelper.toObjectMap(cosignatureContainer.get("cosignature"));
			cosignatureDescriptor.put("type", "cosignature_v1");

			wrapped.add(CatbufferVectorsHelper.wrapNemCosignature(facade, cosignatureDescriptor));
		}
		descriptor.put("cosignatures", wrapped);
	}

	// endregion

	// region Symbol transactions

	@TestFactory
	Iterable<DynamicTest> descriptorSymbolTransactions() {
		final SymbolFacade facade = new SymbolFacade("testnet");
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("transactions"),
				item -> assertCreateSymbolTransactionFromDescriptor(facade, item));
	}

	private static void assertCreateSymbolTransactionFromDescriptor(final SymbolFacade facade, final Map<String, Object> item) {
		final Map<String, Object> descriptor = CatbufferDescriptorHelper.normalizeInput((Map<?, ?>) item.get("descriptor"));
		CatbufferDescriptorHelper.fixupDescriptorCommon(descriptor);
		fixupSymbolTransactionDescriptor(facade, descriptor);

		final Serializer transaction = facade.transactionFactory.create(descriptor);
		assertSchemaType(item, transaction);
		CatbufferVectorsHelper.assertPayload(item, transaction, "descriptor");
		assertConversions(descriptor, transaction);
	}

	private static void fixupSymbolTransactionDescriptor(final SymbolFacade facade, final Map<String, Object> descriptor) {

		replaceListField(descriptor, "transactions",
				element -> facade.transactionFactory.createEmbedded(CatbufferVectorsHelper.toObjectMap(element)));
		replaceListField(descriptor, "cosignatures",
				element -> CatbufferVectorsHelper.toSymbolCosignature(CatbufferVectorsHelper.toObjectMap(element)));
	}

	// endregion

	// region Symbol blocks

	@TestFactory
	Iterable<DynamicTest> descriptorSymbolBlocks() {
		final SymbolFacade facade = new SymbolFacade("testnet");
		final SymbolBlockFactory blockFactory = new SymbolBlockFactory(facade.network);
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("blocks"),
				item -> assertCreateSymbolBlockFromDescriptor(facade, blockFactory, item));
	}

	private static void assertCreateSymbolBlockFromDescriptor(final SymbolFacade facade, final SymbolBlockFactory blockFactory,
			final Map<String, Object> item) {
		final Map<String, Object> descriptor = CatbufferDescriptorHelper.normalizeInput((Map<?, ?>) item.get("descriptor"));
		CatbufferDescriptorHelper.fixupDescriptorCommon(descriptor);

		final Object transactions = descriptor.get("transactions");
		if (transactions instanceof List<?> list) {
			final List<org.symbol.sdk.symbol.models.Transaction> built = new ArrayList<>();
			for (Object element : list) {
				final Map<String, Object> child = CatbufferVectorsHelper.toObjectMap(element);
				fixupSymbolTransactionDescriptor(facade, child);
				built.add(facade.transactionFactory.create(child));
			}
			descriptor.put("transactions", built);
		}

		final Serializer block = blockFactory.create(descriptor);
		assertSchemaType(item, block);
		CatbufferVectorsHelper.assertPayload(item, block, "descriptor");
		assertConversions(descriptor, block);
	}

	// endregion

	// region Symbol receipts

	@TestFactory
	Iterable<DynamicTest> descriptorSymbolReceipts() {
		final SymbolReceiptFactory receiptFactory = new SymbolReceiptFactory();
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("receipts"),
				item -> assertCreateSymbolReceiptFromDescriptor(receiptFactory, item));
	}

	private static void assertCreateSymbolReceiptFromDescriptor(final SymbolReceiptFactory receiptFactory, final Map<String, Object> item) {
		// receipts have no nested hex strings or signature, so key/number normalization is all
		// the descriptor needs.
		final Map<String, Object> descriptor = CatbufferDescriptorHelper.normalizeInput((Map<?, ?>) item.get("descriptor"));

		final Serializer receipt = receiptFactory.create(descriptor);
		assertSchemaType(item, receipt);
		CatbufferVectorsHelper.assertPayload(item, receipt, "descriptor");
		assertConversions(descriptor, receipt);
	}

	// endregion

	// region helpers

	/** Anchors the created entity's concrete class to the vector's {@code schema_name} (JS's by-name lookups make this implicit). */
	private static void assertSchemaType(final Map<String, Object> item, final Serializer created) {
		assertThat("created type must match vector schema_name for " + item.get("test_name"), created.getClass().getSimpleName(),
				equalTo(item.get("schema_name")));
	}

	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	/**
	 * Asserts every key of the (fixed-up) descriptor is present in both the entity's {@code toString()} text and its {@code toJson()} map,
	 * and that the {@code toJson()} map serializes to a JSON document without throwing.
	 */
	private static void assertConversions(final Map<String, Object> descriptor, final Serializer entity) {
		final String formatted = entity.toString();
		final Map<String, Object> json = CatbufferVectorsHelper.toObjectMap(((org.symbol.sdk.CatbufferType) entity).toJson());

		for (String key : descriptor.keySet()) {
			if (canSkipMissingParentName(entity, key))
				continue;

			assertThat("descriptor key '" + key + "' missing from toString() of " + entity.getClass().getSimpleName(), formatted,
					containsString(key));
			assertThat("descriptor key '" + key + "' missing from toJson() of " + entity.getClass().getSimpleName(), json, hasKey(key));
		}

		assertDoesNotThrow(() -> JSON_MAPPER.writeValueAsString(json),
				"toJson() is not JSON-serializable for " + entity.getClass().getSimpleName());
	}

	/**
	 * A {@code parentName} descriptor key is allowed to be absent from the projections when the entity's parentName is null (NEM namespace
	 * registration renders it as a nullable conditional field). Read through the descriptor-pipeline {@code getField} — the key only
	 * appears on descriptors whose entity carries the field, so no reflective probing is needed.
	 */
	private static boolean canSkipMissingParentName(final Serializer entity, final String key) {
		return "parentName".equals(key) && null == ((org.symbol.sdk.CatbufferType) entity).getField("parentName");
	}

	private static <T> void replaceListField(final Map<String, Object> descriptor, final String key, final Function<Object, T> mapper) {
		final Object value = descriptor.get(key);
		if (!(value instanceof List<?> list))
			return;

		final List<T> mapped = new ArrayList<>(list.size());
		for (Object element : list)
			mapped.add(mapper.apply(element));

		descriptor.put(key, mapped);
	}

	// endregion
}
