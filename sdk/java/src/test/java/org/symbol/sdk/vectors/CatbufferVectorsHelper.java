package org.symbol.sdk.vectors;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.equalToIgnoringCase;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.function.Consumer;

import org.junit.jupiter.api.DynamicTest;

import org.symbol.sdk.Serializer;
import org.symbol.sdk.facade.NemFacade;
import org.symbol.sdk.utils.Converter;

/**
 * Shared helpers for catbuffer vector tests: the memoized {@code SCHEMAS_PATH}-aware test-case loader, the checked JSON-tree map copy, the
 * payload assertion, and the cosignature model builders shared by the plain-descriptor and typed-descriptor suites.
 */
final class CatbufferVectorsHelper {

	// Default magnitude-based typing (Integer/Long, BigInteger only above Long.MAX_VALUE) matching the production JsonDescriptor
	// mapper; CatbufferDescriptorHelper.normalizeInput narrows every Number to the long-backed value the models expect.
	private static final com.fasterxml.jackson.databind.ObjectMapper JSON_MAPPER = new com.fasterxml.jackson.databind.ObjectMapper();

	private CatbufferVectorsHelper() {
	}

	/** Parses a vectors JSON file into a plain value tree. */
	private static Object parseJsonFile(final Path file) {
		try {
			return JSON_MAPPER.readValue(Files.readString(file, StandardCharsets.UTF_8), Object.class);
		} catch (final IOException ex) {
			throw new UncheckedIOException(ex);
		}
	}

	/**
	 * Checked copy of a parsed JSON object into a descriptor-shaped map; replaces unchecked {@code (Map<String, Object>)} casts of
	 * JSON-tree values.
	 *
	 * @param value Parsed JSON value whose top level is an object.
	 * @return Mutable map with checked {@code String} keys.
	 */
	/**
	 * Converts each element of a JSON array into a checked object map (see {@link #toObjectMap}).
	 *
	 * @param values JSON array elements.
	 * @return One object map per element.
	 */
	static List<Map<String, Object>> toObjectMaps(final List<?> values) {
		final List<Map<String, Object>> maps = new ArrayList<>(values.size());
		for (final Object value : values)
			maps.add(toObjectMap(value));

		return maps;
	}

	static Map<String, Object> toObjectMap(final Object value) {
		if (!(value instanceof Map<?, ?> map))
			throw new IllegalArgumentException("expected JSON object map but got " + (null == value ? "null" : value.getClass().getName()));

		final Map<String, Object> result = new LinkedHashMap<>();
		map.forEach((key, nestedValue) -> result.put((String) key, nestedValue));
		return result;
	}

	// shared across the @TestFactory methods (catVectors runs in one JVM, no forkEvery); entries are deep-unmodifiable so a
	// consumer that mutates without first deep-copying via CatbufferDescriptorHelper.normalizeInput fails fast instead of
	// poisoning sibling suites
	private static final ConcurrentHashMap<String, List<Map<String, Object>>> CASE_CACHE = new ConcurrentHashMap<>();

	/**
	 * Loads test cases from {@code ${SCHEMAS_PATH}/{blockchain}/models/*.json}, applying optional filename include/exclude filters;
	 * memoized per filter combination so sibling test classes do not re-parse the same corpus files.
	 *
	 * @param blockchain Blockchain folder name ({@code "nem"} or {@code "symbol"}).
	 * @param includes When non-null, only files whose name contains at least one substring are loaded.
	 * @param excludes When non-null, files whose name contains any substring are dropped.
	 * @return Loaded test cases in deterministic filename order.
	 */
	static List<Map<String, Object>> prepareTestCases(final String blockchain, final List<String> includes, final List<String> excludes) {
		final String cacheKey = blockchain + "|" + includes + "|" + excludes;
		return CASE_CACHE.computeIfAbsent(cacheKey, unused -> deepUnmodifiableCases(loadTestCases(blockchain, includes, excludes)));
	}

	/**
	 * Builds one {@link DynamicTest} per loaded vector case, named by its {@code test_name}, running {@code body} on the case.
	 *
	 * @param blockchain Blockchain folder name.
	 * @param includes Optional filename include filters.
	 * @param body Per-case assertion.
	 * @return One dynamic test per case.
	 */
	static Iterable<DynamicTest> perCaseTests(final String blockchain, final List<String> includes,
			final Consumer<Map<String, Object>> body) {
		final List<DynamicTest> tests = new ArrayList<>();
		for (final Map<String, Object> item : prepareTestCases(blockchain, includes, null))
			tests.add(DynamicTest.dynamicTest((String) item.get("test_name"), () -> body.accept(item)));

		return tests;
	}

	private static List<Map<String, Object>> deepUnmodifiableCases(final List<Map<String, Object>> cases) {
		final List<Map<String, Object>> copy = new ArrayList<>(cases.size());
		for (final Map<String, Object> item : cases) {
			final Map<String, Object> itemCopy = new LinkedHashMap<>();
			item.forEach((key, value) -> itemCopy.put(key, deepUnmodifiable(value)));
			copy.add(Collections.unmodifiableMap(itemCopy));
		}

		return Collections.unmodifiableList(copy);
	}

	private static Object deepUnmodifiable(final Object value) {
		if (value instanceof Map<?, ?> map) {
			final Map<Object, Object> copy = new LinkedHashMap<>();
			map.forEach((key, nested) -> copy.put(key, deepUnmodifiable(nested)));
			return Collections.unmodifiableMap(copy);
		}

		if (value instanceof List<?> list) {
			final List<Object> copy = new ArrayList<>(list.size());
			list.forEach(element -> copy.add(deepUnmodifiable(element)));
			return Collections.unmodifiableList(copy);
		}

		return value;
	}

	private static List<Map<String, Object>> loadTestCases(final String blockchain, final List<String> includes,
			final List<String> excludes) {
		String schemasRoot = System.getenv("SCHEMAS_PATH");
		if (schemasRoot == null || schemasRoot.isBlank())
			schemasRoot = ".";

		final Path schemasPath = Paths.get(schemasRoot, blockchain, "models");

		if (!Files.exists(schemasPath))
			throw new IllegalStateException("could not find any cases because " + schemasPath + " does not exist");

		// Sort filenames for determinism (matches python pytest collection order more closely).
		final List<Path> jsonFiles = new ArrayList<>();
		try (final DirectoryStream<Path> stream = Files.newDirectoryStream(schemasPath, "*.json")) {
			stream.forEach(jsonFiles::add);
		} catch (IOException ex) {
			throw new UncheckedIOException(ex);
		}
		Collections.sort(jsonFiles);

		final List<Map<String, Object>> cases = new ArrayList<>();
		for (final Path file : jsonFiles) {
			final String name = file.getFileName().toString();
			if (shouldSkipByFilters(name, includes, excludes))
				continue;

			final Object parsed = parseJsonFile(file);
			if (!(parsed instanceof List<?> entries))
				throw new IllegalStateException(name + " did not parse to a JSON array");

			for (final Object entry : entries)
				cases.add(toObjectMap(entry));
		}

		if (cases.isEmpty())
			throw new IllegalStateException("could not find any cases in " + schemasPath);

		return cases;
	}

	private static boolean shouldSkipByFilters(final String fileName, final List<String> includes, final List<String> excludes) {
		if (includes != null && includes.stream().noneMatch(fileName::contains)) {
			System.out.println("skipping " + fileName + " due to include filters");
			return true;
		}

		if (excludes != null && excludes.stream().anyMatch(fileName::contains)) {
			System.out.println("skipping " + fileName + " due to exclude filters");
			return true;
		}

		return false;
	}

	/**
	 * Asserts the entity serializes byte-for-byte to the vector payload.
	 *
	 * @param item Vector test case.
	 * @param entity Entity under test.
	 * @param label Suite label used in the failure message.
	 */
	static void assertPayload(final Map<String, Object> item, final Serializer entity, final String label) {
		final String expectedHex = (String) item.get("payload");
		final String actualHex = Converter.uint8ToHex(entity.serialize());
		assertThat(label + " payload mismatch for " + item.get("test_name"), actualHex, equalToIgnoringCase(expectedHex));
	}

	/**
	 * Creates a NEM cosignature transaction from a fully-prepared create descriptor and wraps it size-prefixed; shared by the
	 * plain-descriptor and typed-descriptor suites.
	 *
	 * @param facade NEM facade.
	 * @param cosignatureDescriptor Cosignature create descriptor (type + fields).
	 * @return Size-prefixed cosignature model.
	 */
	static org.symbol.sdk.nem.models.SizePrefixedCosignatureV1 wrapNemCosignature(final NemFacade facade,
			final Map<String, Object> cosignatureDescriptor) {
		final org.symbol.sdk.nem.models.CosignatureV1 cosignature = (org.symbol.sdk.nem.models.CosignatureV1) facade.transactionFactory
				.create(cosignatureDescriptor);
		// fixup for a network-field mismatch in the vectors themselves
		cosignature.setNetwork(org.symbol.sdk.nem.models.NetworkType.MAINNET);

		final org.symbol.sdk.nem.models.SizePrefixedCosignatureV1 sized = new org.symbol.sdk.nem.models.SizePrefixedCosignatureV1();
		sized.setCosignature(cosignature);
		return sized;
	}

	/**
	 * Builds a symbol cosignature model from a vector cosignature descriptor; the pod {@code parse} calls accept both the typed suite's hex
	 * strings and the plain-descriptor suite's raw bytes.
	 *
	 * @param cosignatureDescriptor Cosignature descriptor.
	 * @return Cosignature model.
	 */
	static org.symbol.sdk.symbol.models.Cosignature toSymbolCosignature(final Map<String, Object> cosignatureDescriptor) {
		final org.symbol.sdk.symbol.models.Cosignature cosignature = new org.symbol.sdk.symbol.models.Cosignature();
		cosignature.setSignature(org.symbol.sdk.symbol.models.Signature.parse(cosignatureDescriptor.get("signature")));
		cosignature.setSignerPublicKey(org.symbol.sdk.symbol.models.PublicKey.parse(cosignatureDescriptor.get("signerPublicKey")));
		if (cosignatureDescriptor.containsKey("version"))
			cosignature.setVersion(Converter.toLong((Number) cosignatureDescriptor.get("version")));

		return cosignature;
	}

}
