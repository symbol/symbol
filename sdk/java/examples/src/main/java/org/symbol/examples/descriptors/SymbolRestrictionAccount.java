package org.symbol.examples.descriptors;

import java.util.List;

/** Sample Symbol account restriction transaction descriptors. */
public final class SymbolRestrictionAccount {
	private SymbolRestrictionAccount() {
	}

	public static List<String> descriptors() {
		return List.of(
				// allow incoming transactions only from address below
				"""
				{
					"type": "account_address_restriction_transaction_v1",
					"restrictionFlags": "address",
					"restrictionAdditions": ["TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y"]
				}
				""",
				// block transactions outgoing to given address. note: block and allow restrictions
				// are mutually exclusive — see https://docs.symbol.dev/concepts/account-restriction.html
				"""
				{
					"type": "account_address_restriction_transaction_v1",
					"restrictionFlags": "address outgoing block",
					"restrictionAdditions": ["TASYMBOLLK6FSL7GSEMQEAWN7VW55ZSZU2Q2Q5Y"]
				}
				""",
				"""
				{
					"type": "account_mosaic_restriction_transaction_v1",
					"restrictionFlags": "mosaic_id",
					"restrictionAdditions": ["0x7EDCBA90FEDCBA90"]
				}
				""",
				// allow only specific transaction types
				"""
				{
					"type": "account_operation_restriction_transaction_v1",
					"restrictionFlags": "outgoing",
					"restrictionAdditions": [
						"transfer",
						"account_key_link",
						"vrf_key_link",
						"voting_key_link",
						"node_key_link"
					]
				}
				""");
	}
}
