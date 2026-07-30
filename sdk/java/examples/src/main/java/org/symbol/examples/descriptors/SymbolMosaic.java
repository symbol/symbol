package org.symbol.examples.descriptors;

import java.util.List;

import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;

/** Sample Symbol mosaic definition and supply transaction descriptors. */
public final class SymbolMosaic {
	private SymbolMosaic() {
	}

	public static List<String> descriptors() {
		final Address sampleAddress = new Address("TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y");

		return List.of(
				"""
				{
					"type": "mosaic_definition_transaction_v1",
					"duration": 1,
					"nonce": 123,
					"flags": "transferable restrictable",
					"divisibility": 2
				}
				""",
				// delta assumes divisibility = 2
				"""
				{
					"type": "mosaic_supply_change_transaction_v1",
					"mosaicId": "0x%X",
					"delta": 100000,
					"action": "increase"
				}
				""".formatted(IdGenerator.generateMosaicId(sampleAddress, 123L)));
	}
}
