package org.symbol.examples.descriptors;

import java.util.List;

/** Sample Symbol hash lock and secret lock/proof transaction descriptors. */
public final class SymbolLock {
	private SymbolLock() {
	}

	public static List<String> descriptors() {
		return List.of(
				// note: only network currency can be used as a mosaic in hash lock
				"""
				{
					"type": "hash_lock_transaction_v1",
					"mosaic": {"mosaicId": "0x7EDCBA90FEDCBA90", "amount": 123000000},
					"duration": 123,
					"hash": "0000000000000000000000000000000000000000000000000000000000000000"
				}
				""",
				"""
				{
					"type": "secret_lock_transaction_v1",
					"mosaic": {"mosaicId": "0x7EDCBA90FEDCBA90", "amount": 123000000},
					"duration": 123,
					"recipientAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"secret": "C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000",
					"hashAlgorithm": "hash_160"
				}
				""",
				// the proof is hex for the four raw bytes hashed into the secret (hash_160(0xC1ECFDFC) == secret); JSON cannot
				// carry raw bytes, so the sign example hex-decodes the proof at the call site — the same unhexlify step the
				// JS / Python examples perform
				"""
				{
					"type": "secret_proof_transaction_v1",
					"recipientAddress": "TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y",
					"secret": "C849C5A5F6BCA84EF1829B2A84C0BAC9D765383D000000000000000000000000",
					"hashAlgorithm": "hash_160",
					"proof": "C1ECFDFC"
				}
				""");
	}
}
