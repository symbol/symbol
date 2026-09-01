//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.time.Instant;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public final class NetworkTime {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://whydah.symbolmain.net:3001");

	public static void main(final String[] args) {
		try {
			new NetworkTime().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// Fetch Nemesis timestamp
		final String propertiesPath = "/network/properties"; // [>step-1]
		System.out.printf(
			"Fetching network properties from %s%n", propertiesPath);
		final HttpRequest propertiesRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + propertiesPath)).GET().build();
		final HttpResponse<String> propertiesResponse =
			HTTP_CLIENT.send(
				propertiesRequest, BodyHandlers.ofString());
		final JsonNode propertiesJson = JSON_MAPPER.readTree(
			propertiesResponse.body());
		final String nemesisStr = propertiesJson.get("network")
			.get("epochAdjustment").asText();
		final long nemesisSeconds =
			Long.parseLong(nemesisStr.replace("s", ""));
		final Instant nemesisDatetime =
			Instant.ofEpochSecond(nemesisSeconds);
		// [<step-1]
		// Fetch current network timestamp
		final String timePath = "/node/time"; // [>step-2]
		System.out.printf(
			"Fetching current network time from %s%n", timePath);
		final HttpRequest timeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + timePath)).GET().build();
		final HttpResponse<String> timeResponse = HTTP_CLIENT.send(
			timeRequest, BodyHandlers.ofString());
		final JsonNode timeJson = JSON_MAPPER.readTree(
			timeResponse.body());
		final long networkMs = timeJson
			.get("communicationTimestamps")
			.get("receiveTimestamp").asLong(); // [<step-2]
		// [>step-3]
		final Instant networkDatetime =
			nemesisDatetime.plusMillis(networkMs);

		System.out.printf("%nNemesis time (UTC): %s%n",
			nemesisDatetime);
		System.out.printf("Network time (ms since Nemesis): %d%n",
			networkMs);
		System.out.printf("Network time (UTC): %s%n",
			networkDatetime); // [<step-3]
	}
}
