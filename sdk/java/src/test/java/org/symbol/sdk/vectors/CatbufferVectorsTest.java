package org.symbol.sdk.vectors;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalTo;
import static org.hamcrest.Matchers.greaterThan;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.notNullValue;

import java.nio.ByteBuffer;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.function.Supplier;

import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.Tag;
import org.junit.jupiter.api.TestFactory;

import org.symbol.sdk.Serializer;
import org.symbol.sdk.symbol.models.AddressResolutionStatement;
import org.symbol.sdk.symbol.models.BlockStatement;
import org.symbol.sdk.symbol.models.DetachedCosignature;
import org.symbol.sdk.symbol.models.FinalizedBlockHeader;
import org.symbol.sdk.symbol.models.MosaicResolutionStatement;
import org.symbol.sdk.symbol.models.TransactionStatement;
import org.symbol.sdk.utils.Converter;

/**
 * Payload-first vector tests, tagged {@code catvectors} ({@code ./gradlew catVectors}; requires {@code SCHEMAS_PATH} pointing at
 * {@code tests/vectors}): every case under {@code tests/vectors/{nem,symbol}/models/*.json} deserializes from its payload and must
 * serialize back byte-for-byte — independent of the descriptor-driven creation in {@link CatbufferDescriptorVectorsTest}. Factory-homed
 * files dispatch through the generated umbrella-factory {@code deserialize} entry points; the factory-less other.json statements use an
 * explicit constructor/deserializer map. The file also pins the default-construction contract for every vector-observed schema — a
 * default-constructed model must report a non-zero size and serialize successfully to exactly that many bytes (the generated
 * ModelsSweepTest serializes only populated instances, so this is the sole default-instance coverage).
 */
@Tag("catvectors")
final class CatbufferVectorsTest {
	// region roundtrip

	/** Constructor and deserializer for a factory-less other.json schema, declared once so the two sweeps cannot drift. */
	private record OtherSchema(Supplier<Serializer> constructor, Function<ByteBuffer, Serializer> deserializer) {
	}

	private static final Map<String, OtherSchema> OTHER_SCHEMAS = Map.of("AddressResolutionStatement",
			new OtherSchema(AddressResolutionStatement::new, AddressResolutionStatement::deserialize), "BlockStatement",
			new OtherSchema(BlockStatement::new, BlockStatement::deserialize), "DetachedCosignature",
			new OtherSchema(DetachedCosignature::new, DetachedCosignature::deserialize), "FinalizedBlockHeader",
			new OtherSchema(FinalizedBlockHeader::new, FinalizedBlockHeader::deserialize), "MosaicResolutionStatement",
			new OtherSchema(MosaicResolutionStatement::new, MosaicResolutionStatement::deserialize), "TransactionStatement",
			new OtherSchema(TransactionStatement::new, TransactionStatement::deserialize));

	@TestFactory
	Iterable<DynamicTest> roundtripNemTransactions() {
		// CosignatureV1 is the one non-Transaction schema; it is not routed through the umbrella factory (mirrors JS)
		return CatbufferVectorsHelper.perCaseTests("nem", List.of("transactions"),
				item -> assertRoundtrip(item,
						"CosignatureV1".equals(item.get("schema_name"))
								? org.symbol.sdk.nem.models.CosignatureV1::deserialize
								: org.symbol.sdk.nem.models.TransactionFactory::deserialize));
	}

	@TestFactory
	Iterable<DynamicTest> roundtripSymbolTransactions() {
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("transactions"),
				item -> assertRoundtrip(item, org.symbol.sdk.symbol.models.TransactionFactory::deserialize));
	}

	@TestFactory
	Iterable<DynamicTest> roundtripSymbolBlocks() {
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("blocks"),
				item -> assertRoundtrip(item, org.symbol.sdk.symbol.models.BlockFactory::deserialize));
	}

	@TestFactory
	Iterable<DynamicTest> roundtripSymbolReceipts() {
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("receipts"),
				item -> assertRoundtrip(item, org.symbol.sdk.symbol.models.ReceiptFactory::deserialize));
	}

	@TestFactory
	Iterable<DynamicTest> roundtripSymbolOther() {
		return CatbufferVectorsHelper.perCaseTests("symbol", List.of("other"), item -> assertRoundtrip(item, otherDeserializer(item)));
	}

	private static Function<ByteBuffer, Serializer> otherDeserializer(final Map<String, Object> item) {
		final String schemaName = (String) item.get("schema_name");
		final OtherSchema schema = OTHER_SCHEMAS.get(schemaName);
		assertThat("no schema registered for " + schemaName, schema, is(notNullValue()));

		return schema.deserializer();
	}

	private static void assertRoundtrip(final Map<String, Object> item, final Function<ByteBuffer, Serializer> deserializer) {
		// Arrange:
		final byte[] payload = Converter.hexToUint8((String) item.get("payload"));

		// Act:
		final Serializer instance = deserializer.apply(ByteBuffer.wrap(payload));

		// Assert: the deserialized type matches the vector schema
		assertThat("deserialized type must match vector schema_name for " + item.get("test_name"), instance.getClass().getSimpleName(),
				is(equalTo(item.get("schema_name"))));
		CatbufferVectorsHelper.assertPayload(item, instance, "roundtrip");
		assertThat("size mismatch for " + item.get("test_name"), instance.size(), is(equalTo(payload.length)));
	}

	// endregion

	// region create from constructor

	@TestFactory
	Iterable<DynamicTest> createFromConstructorNem() {
		return createFactoryConstructorTests("nem", null, org.symbol.sdk.nem.models.TransactionFactory::createByName);
	}

	@TestFactory
	Iterable<DynamicTest> createFromConstructorSymbolTransactions() {
		return createFactoryConstructorTests("symbol", List.of("transactions"),
				org.symbol.sdk.symbol.models.TransactionFactory::createByName);
	}

	@TestFactory
	Iterable<DynamicTest> createFromConstructorSymbolBlocks() {
		return createFactoryConstructorTests("symbol", List.of("blocks"), org.symbol.sdk.symbol.models.BlockFactory::createByName);
	}

	@TestFactory
	Iterable<DynamicTest> createFromConstructorSymbolReceipts() {
		return createFactoryConstructorTests("symbol", List.of("receipts"), org.symbol.sdk.symbol.models.ReceiptFactory::createByName);
	}

	@TestFactory
	Iterable<DynamicTest> createFromConstructorSymbolOther() {
		return generateConstructorTests("symbol", List.of("other"), CatbufferVectorsTest::constructorFromOtherSchema);
	}

	private static Iterable<DynamicTest> createFactoryConstructorTests(final String blockchain, final List<String> includes,
			final Function<String, Serializer> createByName) {
		return generateConstructorTests(blockchain, includes, schemaName -> createByName.apply(toUnderscoreName(schemaName)));
	}

	private static Serializer constructorFromOtherSchema(final String schemaName) {
		final OtherSchema schema = OTHER_SCHEMAS.get(schemaName);
		assertThat("no schema registered for " + schemaName, schema, is(notNullValue()));

		return schema.constructor().get();
	}

	private static Iterable<DynamicTest> generateConstructorTests(final String blockchain, final List<String> includes,
			final Function<String, Serializer> constructor) {
		return CatbufferVectorsHelper.prepareTestCases(blockchain, includes, null).stream().map(item -> (String) item.get("schema_name"))
				.distinct().map(name -> DynamicTest.dynamicTest(name, () -> assertCreateFromConstructor(name, constructor))).toList();
	}

	private static void assertCreateFromConstructor(final String schemaName, final Function<String, Serializer> constructor) {
		// Act:
		final Serializer instance = constructor.apply(schemaName);
		final int size = instance.size();
		final byte[] bytes = instance.serialize();

		// Assert:
		assertThat("size==0 for " + schemaName, size, greaterThan(0));
		assertThat("serialize length==0 for " + schemaName, bytes.length, greaterThan(0));
		assertThat("size != serialize length for " + schemaName, size, equalTo(bytes.length));
	}

	/** Converts a CamelCase schema name to the underscore_name the generated {@code createByName} factories are keyed by. */
	private static String toUnderscoreName(final String name) {
		final StringBuilder builder = new StringBuilder(name.length() + 8);
		for (int i = 0; i < name.length(); ++i) {
			final char ch = name.charAt(i);
			if (Character.isUpperCase(ch) && 0 != i)
				builder.append('_');

			builder.append(Character.toLowerCase(ch));
		}

		return builder.toString();
	}

	// endregion
}
