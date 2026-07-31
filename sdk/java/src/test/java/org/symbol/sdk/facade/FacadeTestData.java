package org.symbol.sdk.facade;

/**
 * Shared transfer-JSON fixtures for the facade tests, so {@code SymbolFacadeTest}, {@code NemFacadeTest} and {@code FacadeFactoryTest}
 * certify the same descriptor documents instead of drifting private copies.
 */
final class FacadeTestData {
	static final String SYMBOL_TRANSFER_JSON = """
			{
				"type": "transfer_transaction_v1",
				"recipientAddress": "AEBAGBAFAYDQQCIKBMGA2DQPCAIREEYUCULBOGA",
				"mosaics": [{"mosaicId": 8589934593, "amount": 1000000}],
				"message": "hello symbol"
			}""";

	static final String NEM_TRANSFER_JSON = """
			{
				"type": "transfer_transaction_v1",
				"recipientAddress": "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C",
				"amount": 5,
				"message": {"messageType": "plain", "message": "hello nem"}
			}""";

	private FacadeTestData() {
	}
}
