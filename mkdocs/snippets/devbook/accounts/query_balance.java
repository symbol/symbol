//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.math.BigInteger;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

final class QueryBalance {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final String NODE_URL = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	// [>step-1]
	/**
	 * Fetch account information by address or public key.
	 */
	private static JsonNode getAccountInfo(
		final String accountIdentifier
	) throws IOException, InterruptedException {
		final String accountPath = String.format(
			"/accounts/%s", accountIdentifier);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + accountPath)).GET().build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());

		if (200 != response.statusCode()) {
			if (404 == response.statusCode())
				System.out.printf(
					"Address does not exist: %s%n", response.body());
			else if (409 == response.statusCode())
				System.out.printf(
					"Address is not properly formatted: %s%n",
					response.body());
			else
				System.out.printf("Unexpected error: %s%n",
					response.body());
			System.exit(1);
		}

		return JSON_MAPPER.readTree(response.body()).get("account");
	} // [<step-1]

	// [>step-2]
	/**
	 * Fetch friendly names for a set of mosaics.
	 */
	private static Map<BigInteger, List<String>> getMosaicNames(
		final List<BigInteger> mosaicIds
	) throws IOException, InterruptedException {
		final ObjectNode requestBody = JSON_MAPPER.createObjectNode();
		final ArrayNode mosaicIdsHex = requestBody.putArray("mosaicIds");
		for (final BigInteger mosaicId : mosaicIds)
			mosaicIdsHex.add(String.format("%016X", mosaicId));
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + "/namespaces/mosaic/names"))
			.header("Content-Type", "application/json")
			.POST(HttpRequest.BodyPublishers.ofString(
				requestBody.toString()))
			.build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		final JsonNode namesInfo = JSON_MAPPER.readTree(response.body());

		// Build a map from mosaic IDs to their names
		final Map<BigInteger, List<String>> namesMap = new HashMap<>();
		for (final JsonNode entry : namesInfo.get("mosaicNames")) {
			final BigInteger mosaicId = new BigInteger(
				entry.get("mosaicId").asText(), 16);
			final List<String> names = new ArrayList<>();
			for (final JsonNode name : entry.get("names"))
				names.add(name.asText());
			namesMap.put(mosaicId, names);
		}
		return namesMap;
	} // [<step-2]

	// [>step-3]
	/**
	 * Fetch information for multiple mosaics in a single request.
	 */
	private static Map<BigInteger, JsonNode> getMosaicsInfo(
		final List<BigInteger> mosaicIds
	) throws IOException, InterruptedException {
		final ObjectNode requestBody = JSON_MAPPER.createObjectNode();
		final ArrayNode mosaicIdsHex = requestBody.putArray("mosaicIds");
		for (final BigInteger mosaicId : mosaicIds)
			mosaicIdsHex.add(String.format("%016X", mosaicId));
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + "/mosaics"))
			.header("Content-Type", "application/json")
			.POST(HttpRequest.BodyPublishers.ofString(
				requestBody.toString()))
			.build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		final JsonNode mosaicsInfo = JSON_MAPPER.readTree(
			response.body());

		// Build a map from mosaic IDs to their properties
		final Map<BigInteger, JsonNode> mosaicsMap = new HashMap<>();
		for (final JsonNode entry : mosaicsInfo) {
			final JsonNode mosaic = entry.get("mosaic");
			final BigInteger mosaicId = new BigInteger(
				mosaic.get("id").asText(), 16);
			mosaicsMap.put(mosaicId, mosaic);
		}
		return mosaicsMap;
	} // [<step-3]

	// [>step-4]
	/**
	 * Format an atomic amount with decimal places.
	 */
	private static String formatAmount(
		final BigInteger amount,
		final int divisibility
	) {
		if (0 == divisibility)
			return amount.toString();

		final BigInteger divisor = BigInteger.TEN.pow(divisibility);
		final BigInteger[] parts = amount.divideAndRemainder(divisor);
		return String.format(
			"%s.%0" + divisibility + "d", parts[0], parts[1]);
	} // [<step-4]

	public static void main(final String[] args) {
		new QueryBalance().run();
	}

	private void run() {
		System.out.printf("Using node %s%n", NODE_URL);

		// The account address to query [>step-5]
		final String address = System.getenv().getOrDefault(
			"ADDRESS", "TBIL6D6RURP45YQRWV6Q7YVWIIPLQGLZQFHWFEQ");
		System.out.printf("Fetching account information from %s%n",
			address);

		try {
			// Get account information
			final JsonNode account = getAccountInfo(address);

			// Display balances for all mosaics the account holds
			final JsonNode accountMosaics = account.get("mosaics");
			if (accountMosaics.isEmpty()) {
				System.out.println("Account holds no mosaics");
			} else {
				System.out.printf("Account holds %d mosaic(s):%n",
					accountMosaics.size());

				// Fetch mosaic properties and names for all mosaics
				final List<BigInteger> mosaicIds = new ArrayList<>();
				for (final JsonNode mosaicEntry : accountMosaics)
					mosaicIds.add(new BigInteger(
						mosaicEntry.get("id").asText(), 16));
				final Map<BigInteger, List<String>> mosaicNames =
					getMosaicNames(mosaicIds);
				final Map<BigInteger, JsonNode> mosaicsInfo =
					getMosaicsInfo(mosaicIds);

				for (final JsonNode mosaicEntry : accountMosaics) {
					final BigInteger mosaicId = new BigInteger(
						mosaicEntry.get("id").asText(), 16);
					final BigInteger balance = new BigInteger(
						mosaicEntry.get("amount").asText());

					// Get mosaic properties
					final JsonNode info = mosaicsInfo.get(mosaicId);
					final int divisibility = info.get("divisibility")
						.asInt();

					// Format and display the balance
					final String formattedBalance = formatAmount(
						balance, divisibility);
					final String mosaicIdHex = String.format(
						"0x%016X", mosaicId);

					// Display mosaic ID and names (if available)
					final List<String> names = mosaicNames.getOrDefault(
						mosaicId, List.of());
					if (names.isEmpty())
						System.out.printf("- Mosaic %s%n", mosaicIdHex);
					else
						System.out.printf("- Mosaic %s (%s)%n",
							mosaicIdHex,
							String.join(", ", names));

					System.out.printf("  Balance: %s%n",
						formattedBalance);
					System.out.printf("  Balance (atomic): %s%n",
						balance);
					System.out.printf("  Divisibility: %d%n",
						divisibility);
				}
			}
		} catch (final Exception ex) {
			System.out.println(ex.getMessage());
		} // [<step-5]
	}
}
