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
import com.fasterxml.jackson.databind.node.ObjectNode;

final class QueryBalance {
	private static final ObjectMapper OBJECT_MAPPER = new ObjectMapper();

	private static final String NODE_URL = System.getenv()
		.getOrDefault("NODE_URL", "https://reference.symboltest.net:3001");

	private QueryBalance() {
	}

	private static HttpRequest jsonPost(
		final String path,
		final List<BigInteger> mosaicIds) {
		final ObjectNode requestBody = OBJECT_MAPPER.createObjectNode();
		final var mosaicIdsHex = requestBody.putArray("mosaicIds");
		for (final BigInteger mosaicId : mosaicIds)
			mosaicIdsHex.add(toMosaicIdHex(mosaicId));

		final String body = requestBody.toString();
		return HttpRequest.newBuilder(URI.create(NODE_URL + path))
			.header("Content-Type", "application/json")
			.POST(HttpRequest.BodyPublishers.ofString(body))
			.build();
	}

	private static JsonNode fetchJson(final HttpRequest request)
		throws IOException, InterruptedException {
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());
		return OBJECT_MAPPER.readTree(response.body());
	}

	// [>step-1]
	/**
	 * Fetch account information by address or public key.
	 */
	private static JsonNode getAccountInfo(
		final String accountIdentifier)
		throws IOException, InterruptedException {
		final String accountPath = "/accounts/" + accountIdentifier;
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + accountPath)).GET().build();
		final HttpResponse<String> response = HttpClient.newHttpClient()
			.send(request, BodyHandlers.ofString());

		if (200 != response.statusCode()) {
			if (404 == response.statusCode())
				System.out.println(
					"Address does not exist: " + response.body());
			else if (409 == response.statusCode())
				System.out.println(
					"Address is not properly formatted: "
						+ response.body());
			else
				System.out.println("Unexpected error: " + response.body());
			System.exit(1);
		}

		return OBJECT_MAPPER.readTree(response.body()).get("account");
	} // [<step-1]

	// [>step-2]
	/**
	 * Fetch friendly names for a set of mosaics.
	 */
	private static Map<BigInteger, List<String>> getMosaicNames(
		final List<BigInteger> mosaicIds)
		throws IOException, InterruptedException {
		final HttpRequest request = jsonPost(
			"/namespaces/mosaic/names", mosaicIds);
		final JsonNode namesInfo = fetchJson(request);

		// Build a map from mosaic IDs to their names
		final Map<BigInteger, List<String>> namesMap = new HashMap<>();
		for (final JsonNode entry : namesInfo.get("mosaicNames")) {
			final BigInteger mosaicId = parseMosaicId(
				entry.get("mosaicId"));
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
		final List<BigInteger> mosaicIds)
		throws IOException, InterruptedException {
		final HttpRequest request = jsonPost("/mosaics", mosaicIds);
		final JsonNode mosaicsInfo = fetchJson(request);

		// Build a map from mosaic IDs to their properties
		final Map<BigInteger, JsonNode> mosaicsMap = new HashMap<>();
		for (final JsonNode entry : mosaicsInfo) {
			final JsonNode mosaic = entry.get("mosaic");
			mosaicsMap.put(parseMosaicId(mosaic.get("id")), mosaic);
		}
		return mosaicsMap;
	} // [<step-3]

	// [>step-4]
	/**
	 * Format an atomic amount with decimal places.
	 */
	private static String formatAmount(
		final BigInteger amount,
		final int divisibility) {
		if (0 == divisibility)
			return amount.toString();

		final BigInteger divisor = BigInteger.TEN.pow(divisibility);
		final BigInteger[] parts = amount.divideAndRemainder(divisor);
		return String.format(
			"%s.%0" + divisibility + "d", parts[0], parts[1]);
	} // [<step-4]

	private static BigInteger parseMosaicId(final JsonNode node) {
		return new BigInteger(node.asText(), 16);
	}

	private static String toMosaicIdHex(final BigInteger mosaicId) {
		return "%016X".formatted(mosaicId);
	}

	public static void main(final String[] args) {
		System.out.println("Using node " + NODE_URL);

		// The account address to query [>step-5]
		final String address = System.getenv().getOrDefault(
			"ADDRESS", "TBIL6D6RURP45YQRWV6Q7YVWIIPLQGLZQFHWFEQ");
		System.out.println("Fetching account information from " + address);

		try {
			// Get account information
			final JsonNode account = getAccountInfo(address);

			// Display balances for all mosaics the account holds
			final JsonNode accountMosaics = account.get("mosaics");
			if (accountMosaics.isEmpty()) {
				System.out.println("Account holds no mosaics");
			} else {
				System.out.println(
					"Account holds " + accountMosaics.size()
						+ " mosaic(s):");

				// Fetch mosaic properties and names for all mosaics
				final List<BigInteger> mosaicIds = new ArrayList<>();
				for (final JsonNode mosaicEntry : accountMosaics)
					mosaicIds.add(parseMosaicId(mosaicEntry.get("id")));
				final Map<BigInteger, List<String>> mosaicNames =
					getMosaicNames(mosaicIds);
				final Map<BigInteger, JsonNode> mosaicsInfo =
					getMosaicsInfo(mosaicIds);

				for (final JsonNode mosaicEntry : accountMosaics) {
					final BigInteger mosaicId = parseMosaicId(
						mosaicEntry.get("id"));
					final BigInteger balance = new BigInteger(
						mosaicEntry.get("amount").asText());

					// Get mosaic properties
					final JsonNode info = mosaicsInfo.get(mosaicId);
					final int divisibility = info.get(
						"divisibility").asInt();

					// Format and display the balance
					final String formattedBalance = formatAmount(
						balance, divisibility);
					final String mosaicIdHex = "0x"
						+ toMosaicIdHex(mosaicId);

					// Display mosaic ID and names (if available)
					final List<String> names = mosaicNames.getOrDefault(
						mosaicId, List.of());
					if (names.isEmpty())
						System.out.println("- Mosaic " + mosaicIdHex);
					else
						System.out.println(
							"- Mosaic " + mosaicIdHex
								+ " (" + String.join(", ", names) + ")");

					System.out.println("  Balance: " + formattedBalance);
					System.out.println("  Balance (atomic): " + balance);
					System.out.println("  Divisibility: " + divisibility);
				}
			}
		} catch (final IOException | InterruptedException ex) {
			System.out.println(ex.getMessage());
		} // [<step-5]
	}
}
