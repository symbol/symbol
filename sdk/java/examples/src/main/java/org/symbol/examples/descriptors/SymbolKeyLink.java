package org.symbol.examples.descriptors;

import java.util.List;

/** Sample Symbol account/node/VRF/voting key link transaction descriptors. */
public final class SymbolKeyLink {
	private SymbolKeyLink() {
	}

	public static List<String> descriptors() {
		return List.of(
				"""
				{
					"type": "account_key_link_transaction_v1",
					"linkedPublicKey": "BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F",
					"linkAction": "link"
				}
				""",
				"""
				{
					"type": "node_key_link_transaction_v1",
					"linkedPublicKey": "BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F",
					"linkAction": "link"
				}
				""",
				"""
				{
					"type": "voting_key_link_transaction_v1",
					"linkedPublicKey": "BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F",
					"linkAction": "link",
					"startEpoch": 10,
					"endEpoch": 150
				}
				""",
				"""
				{
					"type": "vrf_key_link_transaction_v1",
					"linkedPublicKey": "BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F",
					"linkAction": "link"
				}
				""");
	}
}
