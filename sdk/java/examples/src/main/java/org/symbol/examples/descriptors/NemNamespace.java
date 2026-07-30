package org.symbol.examples.descriptors;

import java.util.List;

/** Sample NEM namespace registration transaction descriptors. */
public final class NemNamespace {
	private NemNamespace() {
	}

	public static List<String> descriptors() {
		return List.of(
				// root namespace
				"""
				{
					"type": "namespace_registration_transaction_v1",
					"rentalFeeSink": "TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35",
					"rentalFee": 50000000000,
					"name": "roger"
				}
				""",
				// child namespace
				"""
				{
					"type": "namespace_registration_transaction_v1",
					"rentalFeeSink": "TAMESPACEWH4MKFMBCVFERDPOOP4FK7MTDJEYP35",
					"rentalFee": 1000000,
					"parentName": "roger",
					"name": "charlie"
				}
				""");
	}
}
