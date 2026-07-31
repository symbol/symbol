package org.symbol.examples.descriptors;

import java.util.List;

import org.symbol.sdk.symbol.IdGenerator;

/** Sample Symbol namespace registration transaction descriptors. */
public final class SymbolNamespace {
	private SymbolNamespace() {
	}

	public static List<String> descriptors() {
		return List.of(
				"""
				{
					"type": "namespace_registration_transaction_v1",
					"registrationType": "root",
					"duration": 123,
					"name": "roger"
				}
				""",
				"""
				{
					"type": "namespace_registration_transaction_v1",
					"registrationType": "child",
					"parentId": "0x%X",
					"name": "charlie"
				}
				""".formatted(IdGenerator.generateNamespaceId("roger")));
	}
}
