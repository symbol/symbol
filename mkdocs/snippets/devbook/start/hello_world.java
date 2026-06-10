import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse.BodyHandlers;
import java.time.Duration;

import com.fasterxml.jackson.databind.ObjectMapper;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.NetworkTimestamp;

class HelloWorld {
	public static void main(String[] args) {
		// [>step-1]
		var facade = new SymbolFacade("mainnet");
		System.out.println(
			"Network name: " + facade.getNetwork().getName());
		// NetworkTimestamp(0) is the genesis block timestamp
		var launchDate = facade.getNetwork().toDatetime(
			new NetworkTimestamp(0));
		System.out.println("Network launch date: " + launchDate); // [<step-1]
		// [>step-2]
		var nodeUrl = "https://reference.symboltest.net:3001";
		System.out.println("Using node " + nodeUrl);
		try {
			// Fetch current chain information
			var infoPath = "/chain/info";
			System.out.println(
				"Fetching chain information from " + infoPath);
			var client = HttpClient.newHttpClient();
			var request = HttpRequest
				.newBuilder(URI.create(nodeUrl + infoPath))
				.timeout(Duration.ofSeconds(10))
				.GET()
				.build();
			var response = client.send(request, BodyHandlers.ofString());
			var responseJson = new ObjectMapper()
				.readTree(response.body());
			var height = responseJson.get("height").asLong();
			System.out.printf("  Blockchain height: %,d blocks%n", height);

		} catch (IOException | InterruptedException e) {
			System.out.println(e.getMessage());
		} // [<step-2]
	}
}
