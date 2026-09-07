//JAVA 21+
//DEPS com.fasterxml.jackson.core:jackson-databind:2.17.1
//DEPS org.glassfish.tyrus.bundles:tyrus-standalone-client:2.2.0

import java.io.IOException;
import java.net.URI;
import java.util.concurrent.CompletableFuture;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.websocket.ClientEndpoint;
import jakarta.websocket.ContainerProvider;
import jakarta.websocket.OnMessage;
import jakarta.websocket.RemoteEndpoint;
import jakarta.websocket.Session;
import jakarta.websocket.WebSocketContainer;

@ClientEndpoint
public final class ListenNewBlocks {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private final CompletableFuture<String> uidFuture =
		new CompletableFuture<>();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final String wsUrl = nodeUrl.replaceFirst("http", "ws")
		+ "/ws";

	public static void main(final String[] args) {
		try {
			new ListenNewBlocks().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws Exception {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		final WebSocketContainer container =
			ContainerProvider.getWebSocketContainer();
		final Session session = container.connectToServer(
			this, URI.create(wsUrl));
		final RemoteEndpoint.Basic remote = session.getBasicRemote();

		// Connect to websocket endpoint
		final String uid = uidFuture.join();
		System.out.printf("Connected to %s with uid %s%n", wsUrl, uid);
		// [<step-1]
		// Subscribe to block channel [>step-2]
		remote.sendText(JSON_MAPPER.createObjectNode()
			.put("uid", uid).put("subscribe", "block")
			.toString());
		System.out.println("Subscribed to block channel");

		// Subscribe to finalizedBlock channel
		remote.sendText(JSON_MAPPER.createObjectNode()
			.put("uid", uid).put("subscribe", "finalizedBlock")
			.toString());
		System.out.println("Subscribed to finalizedBlock channel");
		// [<step-2]
		// Unsubscribe on exit [>step-4]
		Runtime.getRuntime().addShutdownHook(new Thread(() -> {
			try {
				remote.sendText(JSON_MAPPER.createObjectNode()
					.put("uid", uid).put("unsubscribe", "block")
					.toString());
				remote.sendText(JSON_MAPPER.createObjectNode()
					.put("uid", uid).put("unsubscribe", "finalizedBlock")
					.toString());
				System.out.println("Unsubscribed from all channels");
				session.close();
			} catch (final IOException ex) {
				throw new IllegalStateException(ex);
			}
		})); // [<step-4]

		// Wait forever
		Thread.currentThread().join();
	}

	// Handle incoming messages [>step-3]
	@OnMessage
	public void onMessage(final String payload) throws IOException {
		final JsonNode message = JSON_MAPPER.readTree(payload);

		// Special case for the initial handshake message
		if (message.has("uid")) {
			uidFuture.complete(message.get("uid").asText());
			return;
		}

		final String topic = message.get("topic").asText();

		if ("block".equals(topic)) {
			final JsonNode block = message.get("data").get("block");
			final JsonNode blockMeta = message.get("data").get("meta");
			System.out.printf("New block: height=%,d hash=%s...%n",
				block.get("height").asLong(),
				blockMeta.get("hash").asText().substring(0, 16));
		}

		if ("finalizedBlock".equals(topic)) {
			final JsonNode finalized = message.get("data");
			System.out.printf("Finalized: height=%,d hash=%s...%n",
				finalized.get("height").asLong(),
				finalized.get("hash").asText().substring(0, 16));
		}
	}
	// [<step-3]
}
