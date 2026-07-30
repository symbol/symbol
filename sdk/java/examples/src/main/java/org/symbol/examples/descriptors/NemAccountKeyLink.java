package org.symbol.examples.descriptors;

import java.util.List;

/** Sample NEM account key link transaction descriptors. */
public final class NemAccountKeyLink {
	private NemAccountKeyLink() {
	}

	public static List<String> descriptors() {
		return List.of(
				"""
				{
					"type": "account_key_link_transaction_v1",
					"linkAction": "link",
					"remotePublicKey": "BE0B4CF546B7B4F4BBFCFF9F574FDA527C07A53D3FC76F8BB7DB746F8E8E0A9F"
				}
				""");
	}
}
