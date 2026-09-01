//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.time.Duration;
import java.time.Instant;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.NetworkTimestamp;

final class HelloWorld {
	private HelloWorld() {
	}

	public static void main(final String[] args) {
		// [>step-1]
		final SymbolFacade facade = new SymbolFacade("mainnet");
		System.out.println(
			"Network name: " + facade.network.name);
		// NetworkTimestamp(0) is the genesis block timestamp
		final Instant launchDate = facade.network.toDatetime(
			new NetworkTimestamp(0));
		System.out.println(
			"Network launch date: " + launchDate); // [<step-1]
		// [>step-2]
		final String nodeUrl = "https://reference.symboltest.net:3001";
		System.out.println("Using node " + nodeUrl);
		try {
			// Fetch current chain information
			final String infoPath = "/chain/info";
			System.out.println(
				"Fetching chain information from " + infoPath);
			final HttpClient client = HttpClient.newHttpClient();
			final HttpRequest request = HttpRequest
				.newBuilder(URI.create(nodeUrl + infoPath))
				.timeout(Duration.ofSeconds(10))
				.GET()
				.build();
			final HttpResponse<String> response = client.send(
				request, BodyHandlers.ofString());
			final JsonNode responseJson = new ObjectMapper()
				.readTree(response.body());
			final long height = responseJson.get("height").asLong();
			System.out.printf(
				"  Blockchain height: %,d blocks%n", height);

		} catch (final IOException | InterruptedException ex) {
			System.out.println(ex.getMessage());
		} // [<step-2]
	}
}
