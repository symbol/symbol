package org.symbol.examples.descriptors;

import java.util.List;

/** Sample Symbol address/mosaic alias transaction descriptors. */
public final class SymbolAlias {
	private SymbolAlias() {
	}

	public static List<String> descriptors() {
		return List.of(
				"""
				{
					"type": "address_alias_transaction_v1",
					"namespaceId": "0xC01DFEE7FEEDDEAD",
					"address": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"aliasAction": "link"
				}
				""",
				"""
				{
					"type": "mosaic_alias_transaction_v1",
					"namespaceId": "0xC01DFEE7FEEDDEAD",
					"mosaicId": "0x7EDCBA90FEDCBA90",
					"aliasAction": "link"
				}
				""");
	}
}
