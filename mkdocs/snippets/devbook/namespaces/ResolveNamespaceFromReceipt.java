//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.HashSet;
import java.util.Set;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;

public final class ResolveNamespaceFromReceipt {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	public static void main(final String[] args) {
		try {
			new ResolveNamespaceFromReceipt().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// Hash of a confirmed tx that used a namespace alias [>step-1]
		final String defaultHash =
			"BA0C65DB752A3BF1B25285540642537ECE8C2CA716577EDF8"
			+ "BF0F8597A85ADC4";
		final String txHash = System.getenv().getOrDefault(
			"TRANSACTION_HASH", defaultHash);
		System.out.printf("Transaction hash: %s%n", txHash);
		// [<step-1]
		// Retrieve the confirmed transaction [>step-2]
		final String txPath = "/transactions/confirmed/" + txHash;
		System.out.printf("Fetching transaction from %s%n", txPath);
		final JsonNode txData = getJson(txPath);

		final String blockHeight = txData.get("meta")
			.get("height").asText();
		System.out.printf("  Block height: %s%n", blockHeight);

		// primaryId is 1-based, meta.index is 0-based
		final int txIndex = txData.get("meta").get("index").asInt();
		final int txPrimary = txIndex + 1;
		System.out.printf(
			"  Transaction index: %d (primaryId: %d)%n",
			txIndex, txPrimary); // [<step-2]
		// [>step-3]
		final String recipientHex = txData.get("transaction")
			.get("recipientAddress").asText();
		final boolean isAddressAlias = Address
			.fromDecodedAddressHexString(recipientHex).isAlias();
		System.out.printf("  Recipient: %s%n", recipientHex);
		System.out.printf("  Is address alias: %s%n",
			isAddressAlias); // [<step-3]
		// [>step-4]
		final Set<String> aliasedMosaics = new HashSet<>();
		final JsonNode mosaics = txData.get("transaction").get("mosaics");
		for (final JsonNode mosaic : mosaics) {
			final long mosaicId = Long.parseUnsignedLong(
				mosaic.get("id").asText(), 16);
			final boolean isAlias = IdGenerator.isMosaicAlias(mosaicId);
			if (isAlias)
				aliasedMosaics.add(mosaic.get("id").asText());
			System.out.printf("  Mosaic: %s%n",
				mosaic.get("id").asText());
			System.out.printf("  Is mosaic alias: %s%n", isAlias);
		}
		// [<step-4]
		// Query address resolution statements
		if (isAddressAlias) { // [>step-5]
			final String addressPath =
				"/statements/resolutions/address?height="
				+ blockHeight;
			System.out.printf(
				"%nFetching address resolutions from %s%n",
				addressPath);
			final JsonNode addressData = getJson(addressPath);

			final JsonNode addressStatements = addressData.get("data");
			System.out.printf("  Found %d resolution statement(s)%n",
				addressStatements.size()); // [<step-5]
			// [>step-6]
			for (final JsonNode item : addressStatements) {
				final JsonNode statement = item.get("statement");
				if (!statement.get("unresolved").asText()
					.equals(recipientHex))
					continue;
				String resolved = null;
				for (final JsonNode entry
					: statement.get("resolutionEntries")) {
					if (entry.get("source").get("primaryId")
						.asInt() > txPrimary)
						break;
					resolved = entry.get("resolved").asText();
				}
				if (null != resolved) {
					final Address address = Address
						.fromDecodedAddressHexString(resolved);
					System.out.println("\nAddress resolution:");
					System.out.printf("  Unresolved:  %s%n",
						statement.get("unresolved").asText());
					System.out.printf("  Resolved:   %s%n", address);
				}
			}
			// [<step-6]
		}
		// Query mosaic resolution statements
		if (!aliasedMosaics.isEmpty()) { // [>step-7]
			final String mosaicPath =
				"/statements/resolutions/mosaic?height="
				+ blockHeight;
			System.out.printf(
				"%nFetching mosaic resolutions from %s%n",
				mosaicPath);
			final JsonNode mosaicData = getJson(mosaicPath);

			final JsonNode mosaicStatements = mosaicData.get("data");
			System.out.printf("  Found %d resolution statement(s)%n",
				mosaicStatements.size());

			for (final JsonNode item : mosaicStatements) {
				final JsonNode statement = item.get("statement");
				if (!aliasedMosaics.contains(
					statement.get("unresolved").asText()))
					continue;
				String resolved = null;
				for (final JsonNode entry
					: statement.get("resolutionEntries")) {
					if (entry.get("source").get("primaryId")
						.asInt() <= txPrimary)
						resolved = entry.get("resolved").asText();
				}
				if (null != resolved) {
					System.out.println("\nMosaic resolution:");
					System.out.printf("  Unresolved: %s%n",
						statement.get("unresolved").asText());
					System.out.printf("  Resolved:   %s%n", resolved);
				}
			}
		} // [<step-7]
	}

	private JsonNode getJson(final String path)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body());
	}
}
