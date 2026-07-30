package org.symbol.examples.descriptors;

import java.util.List;

/** Sample Symbol transfer transaction descriptors. */
public final class SymbolTransfer {
	private SymbolTransfer() {
	}

	public static List<String> descriptors() {
		return List.of(
				// mosaics but no message
				"""
				{
					"type": "transfer_transaction_v1",
					"recipientAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"mosaics": [
						{"mosaicId": "0x7EDCBA90FEDCBA90", "amount": 12345000000}
					]
				}
				""",
				// message but no mosaics
				"""
				{
					"type": "transfer_transaction_v1",
					"recipientAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"message": "Wayne Gretzky"
				}
				""",
				// mosaics and message
				"""
				{
					"type": "transfer_transaction_v1",
					"recipientAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"mosaics": [
						{"mosaicId": "0x7EDCBA90FEDCBA90", "amount": 12345000000}
					],
					"message": "You miss 100%% of the shots you don\u2019t take"
				}
				""");
	}
}
