//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public final class ChainHeights {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	public static void main(final String[] args) {
		try {
			new ChainHeights().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		Long prevHeight = null;
		Long prevFinalizedHeight = null;
		Long heightChangedAt = null;
		Long finalizedChangedAt = null;

		while (true) {
			// [>step-1]
			final HttpRequest request = HttpRequest.newBuilder(
				URI.create(nodeUrl + "/chain/info")).GET().build();
			final HttpResponse<String> response = HTTP_CLIENT.send(
				request, BodyHandlers.ofString());
			final JsonNode chainInfo = JSON_MAPPER.readTree(
				response.body());

			final long height = chainInfo.get("height").asLong();
			final JsonNode finalized =
				chainInfo.get("latestFinalizedBlock");
			final long finalizedHeight =
				finalized.get("height").asLong();
			// [<step-1]
			final long now = System.currentTimeMillis();
			// [>step-2]
			if (null != prevHeight && height != prevHeight)
				heightChangedAt = now;
			if (null != prevFinalizedHeight
				&& finalizedHeight != prevFinalizedHeight)
				finalizedChangedAt = now;

			final String heightAgo = null != heightChangedAt
				? "%ds ago".formatted((now - heightChangedAt) / 1000)
				: "-";
			final String finalizedAgo = null != finalizedChangedAt
				? "%ds ago".formatted((now - finalizedChangedAt) / 1000)
				: "-"; // [<step-2]
			// [>step-3]
			System.out.printf(
				"Height: %,10d  (changed %s)"
					+ "  |  Finalized: %,10d  (changed %s)%n",
				height, heightAgo, finalizedHeight, finalizedAgo);

			prevHeight = height;
			prevFinalizedHeight = finalizedHeight;
			Thread.sleep(1000);
			// [<step-3]
		}
	}
}
