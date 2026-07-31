package org.symbol.examples.descriptors;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

/** Sample Symbol account/mosaic/namespace metadata transaction descriptors. */
public final class SymbolMetadata {
	private SymbolMetadata() {
	}

	// Locale.ROOT keeps %d ASCII on locales with non-Latin digits; values are spliced into the JSON via %s unescaped,
	// so they must stay free of quotes, backslashes and control characters
	private static List<String> valueSetAndTrim(final String template, final String value) {
		return List.of(String.format(Locale.ROOT, template, value.length(), value),
				String.format(Locale.ROOT, template, -5, value.substring(0, value.length() - 5)));
	}

	public static List<String> descriptors() {
		final List<String> result = new ArrayList<>();
		result.addAll(valueSetAndTrim("""
				{
					"type": "account_metadata_transaction_v1",
					"targetAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"scopedMetadataKey": "0xC0FFE",
					"valueSizeDelta": %d,
					"value": "%s"
				}
				""", "much coffe, such wow"));
		result.addAll(valueSetAndTrim("""
				{
					"type": "mosaic_metadata_transaction_v1",
					"targetMosaicId": "0x7EDCBA90FEDCBA90",
					"targetAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"scopedMetadataKey": "0xFACADE",
					"valueSizeDelta": %d,
					"value": "%s"
				}
				""", "Once upon a midnight dreary"));
		result.addAll(valueSetAndTrim("""
				{
					"type": "namespace_metadata_transaction_v1",
					"targetNamespaceId": "0xC01DFEE7FEEDDEAD",
					"targetAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"scopedMetadataKey": "0xC1CADA",
					"valueSizeDelta": %d,
					"value": "%s"
				}
				""", "while I pondered, weak and weary"));
		return result;
	}
}
