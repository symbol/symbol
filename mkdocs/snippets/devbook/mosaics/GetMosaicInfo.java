//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.math.BigInteger;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpRequest.BodyPublishers;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.ArrayList;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import org.symbol.sdk.symbol.models.MosaicFlags;

public final class GetMosaicInfo {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private static final String NODE_URL = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");
	private static final String MOSAIC_ID = System.getenv().getOrDefault(
		"MOSAIC_ID", "72C0212E67A08BCE");

	public static void main(final String[] args) {
		try {
			new GetMosaicInfo().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", NODE_URL);
		System.out.printf("Mosaic ID: %s%n", MOSAIC_ID);

		// Fetch mosaic information [>step-1]
		final String mosaicPath = "/mosaics/" + MOSAIC_ID;
		System.out.printf("Fetching mosaic information from %s%n",
			mosaicPath);

		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(NODE_URL + mosaicPath))
			.GET()
			.build();

		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		final JsonNode responseJson =
			JSON_MAPPER.readTree(response.body());
		final JsonNode mosaic = responseJson.get("mosaic");

		System.out.println("Mosaic information:");
		System.out.printf("  Mosaic ID: %s%n", mosaic.get("id").asText());
		System.out.printf("  Supply: %s%n", mosaic.get("supply").asText());
		final int divisibility = mosaic.get("divisibility").asInt();
		System.out.printf("  Divisibility: %d%n", divisibility);

		final int flagsValue = mosaic.get("flags").asInt();
		final MosaicFlags flags = MosaicFlags.fromValue(flagsValue);
		final String flagNames = flags.toString()
			.replace("MosaicFlags.", "")
			.toLowerCase();
		System.out.printf("  Flags: %d (%s)%n", flagsValue, flagNames);
		System.out.printf("  Duration: %s%n",
			mosaic.get("duration").asText());
		System.out.printf("  Start height: %s%n",
			mosaic.get("startHeight").asText());
		System.out.printf("  Revision: %s%n",
			mosaic.get("revision").asText());
		// [<step-1]

		// Display formatted supply [>step-2]
		final BigInteger supply = new BigInteger(
			mosaic.get("supply").asText());
		final BigInteger divisor = BigInteger.TEN.pow(divisibility);
		final BigInteger[] parts = supply.divideAndRemainder(divisor);
		final String fractional = String.format(
			"%0" + divisibility + "d", parts[1]);
		final String formatted =
			String.format("%s.%s", parts[0], fractional);
		System.out.printf("%nSupply in whole units: %s%n", formatted);
		// [<step-2]

		// Fetch namespace names linked to the mosaic [>step-3]
		System.out.printf("%nFetching namespace names for mosaic %s%n",
			MOSAIC_ID);
		final ObjectNode requestBody = JSON_MAPPER.createObjectNode();
		final ArrayNode mosaicIds = requestBody.putArray("mosaicIds");
		mosaicIds.add(MOSAIC_ID);

		final HttpRequest nsRequest = HttpRequest.newBuilder(
			URI.create(NODE_URL + "/namespaces/mosaic/names"))
			.header("Content-Type", "application/json")
			.POST(BodyPublishers.ofString(requestBody.toString()))
			.build();

		final HttpResponse<String> nsResponse = HTTP_CLIENT.send(
			nsRequest, BodyHandlers.ofString());
		final JsonNode namesInfo = JSON_MAPPER.readTree(nsResponse.body());

		for (final JsonNode entry : namesInfo.get("mosaicNames")) {
			final JsonNode names = entry.get("names");
			if (!names.isEmpty()) {
				final List<String> aliases = new ArrayList<>();
				for (final JsonNode name : names)
					aliases.add(name.asText());
				System.out.printf("  Namespace aliases: %s%n",
					String.join(", ", aliases));
			} else {
				System.out.println("  No namespace aliases linked");
			}
		}
		// [<step-3]
	}
}
