package org.symbol.examples.descriptors;

import java.util.List;

/** Sample NEM transfer transaction descriptors. */
public final class NemTransfer {
	private NemTransfer() {
	}

	public static List<String> descriptors() {
		return List.of(
				// mosaics but no message
				"""
				{
					"type": "transfer_transaction_v2",
					"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
					"amount": 12345000000
				}
				""",
				// message but no mosaics
				"""
				{
					"type": "transfer_transaction_v2",
					"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
					"message": {"messageType": "plain", "message": "You miss 100%% of the shots you don\u2019t take"}
				}
				""",
				// mosaics and message
				"""
				{
					"type": "transfer_transaction_v2",
					"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
					"amount": 12345000000,
					"message": {"messageType": "plain", "message": " Wayne Gretzky"}
				}
				""",
				// mosaic bags
				"""
				{
					"type": "transfer_transaction_v2",
					"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
					"amount": 1000000,
					"message": {"messageType": "plain", "message": " Wayne Gretzky"},
					"mosaics": [
						{"mosaic": {
							"mosaicId": {"namespaceId": {"name": "nem"}, "name": "xem"},
							"amount": 12345000000}},
						{"mosaic": {
							"mosaicId": {"namespaceId": {"name": "magic"}, "name": "some_mosaic_with_divisibility_2"},
							"amount": 500}}
					]
				}
				""",
				// mosaics and message V1
				"""
				{
					"type": "transfer_transaction_v1",
					"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
					"amount": 12345000000,
					"message": {"messageType": "plain", "message": " Wayne Gretzky"}
				}
				""");
	}
}
