package org.symbol.examples.descriptors;

import java.util.List;

/** Sample NEM mosaic definition and supply transaction descriptors. */
public final class NemMosaic {
	private NemMosaic() {
	}

	public static List<String> descriptors() {
		return List.of(
				// without properties
				"""
				{
					"type": "mosaic_definition_transaction_v1",
					"rentalFeeSink": "TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC",
					"rentalFee": 50000000000,
					"mosaicDefinition": {
						"ownerPublicKey": "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF",
						"id": {"namespaceId": {"name": "genes"}, "name": "memes"},
						"description": "Not really valuable mosaic",
						"properties": [],
						"levy": {}
					}
				}
				""",
				// with properties
				"""
				{
					"type": "mosaic_definition_transaction_v1",
					"rentalFeeSink": "TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC",
					"rentalFee": 50000000000,
					"mosaicDefinition": {
						"ownerPublicKey": "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF",
						"id": {"namespaceId": {"name": "genes"}, "name": "memes"},
						"description": "Not really valuable mosaic",
						"properties": [
							{"property": {"name": "divisibility", "value": "3"}},
							{"property": {"name": "initialSupply", "value": "123_000"}},
							{"property": {"name": "supplyMutable", "value": "false"}},
							{"property": {"name": "transferable", "value": "true"}}
						],
						"levy": {}
					}
				}
				""",
				// with levy
				"""
				{
					"type": "mosaic_definition_transaction_v1",
					"rentalFeeSink": "TBMOSAICOD4F54EE5CDMR23CCBGOAM2XSJBR5OLC",
					"rentalFee": 50000000000,
					"mosaicDefinition": {
						"ownerPublicKey": "00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF",
						"id": {"namespaceId": {"name": "genes"}, "name": "memes"},
						"description": "Not really valuable mosaic",
						"properties": [],
						"levy": {
							"transferFeeType": "absolute",
							"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
							"mosaicId": {"namespaceId": {"name": "lieutenant"}, "name": "colonel"},
							"fee": 6320000
						}
					}
				}
				""",
				// supply change
				"""
				{
					"type": "mosaic_supply_change_transaction_v1",
					"mosaicId": {"namespaceId": {"name": "genes"}, "name": "memes"},
					"action": "increase",
					"delta": 321000
				}
				""");
	}
}
