package org.symbol.examples.descriptors;

import java.util.List;

/** Sample Symbol mosaic restriction transaction descriptors. */
public final class SymbolRestrictionMosaic {
	private SymbolRestrictionMosaic() {
	}

	public static List<String> descriptors() {
		return List.of(
				"""
				{
					"type": "mosaic_global_restriction_transaction_v1",
					"mosaicId": "0x7EDCBA90FEDCBA90",
					"referenceMosaicId": 0,
					"restrictionKey": "0x0A0D474E5089",
					"previousRestrictionValue": 0,
					"newRestrictionValue": 2,
					"previousRestrictionType": 0,
					"newRestrictionType": "ge"
				}
				""",
				"""
				{
					"type": "mosaic_address_restriction_transaction_v1",
					"mosaicId": "0x7EDCBA90FEDCBA90",
					"restrictionKey": "0x0A0D474E5089",
					"previousRestrictionValue": 0,
					"newRestrictionValue": 5,
					"targetAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y"
				}
				""");
	}
}
