//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1
//DEPS org.glassfish.tyrus.bundles:tyrus-standalone-client:2.2.0

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.List;
import java.util.concurrent.CompletableFuture;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.websocket.ClientEndpoint;
import jakarta.websocket.ContainerProvider;
import jakarta.websocket.OnMessage;
import jakarta.websocket.RemoteEndpoint;
import jakarta.websocket.Session;
import jakarta.websocket.WebSocketContainer;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

@ClientEndpoint
public final class ListenTransactionFlow {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final CompletableFuture<String> uidFuture =
		new CompletableFuture<>();

	private final CompletableFuture<Void> confirmed =
		new CompletableFuture<>();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final String wsUrl = nodeUrl.replaceFirst("http", "ws")
		+ "/ws";

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private String transactionHash;

	public static void main(final String[] args) {
		try {
			new ListenTransactionFlow().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws Exception {
		System.out.printf("Using node %s%n", nodeUrl);
		// [>step-1]
		final String monitorAddress = System.getenv().getOrDefault(
			"MONITOR_ADDRESS",
			"TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I");
		System.out.printf("Monitoring address: %s%n", monitorAddress);

		final String signerPrivateKey = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(signerPrivateKey)); // [<step-1]

		// Connect to WebSocket [>step-2]
		final WebSocketContainer container =
			ContainerProvider.getWebSocketContainer();
		final Session session = container.connectToServer(
			this, URI.create(wsUrl));
		final RemoteEndpoint.Basic remote = session.getBasicRemote();
		final String uid = uidFuture.join();
		System.out.printf("Connected to %s with uid %s%n", wsUrl, uid);
		// [<step-2]
		// Subscribe to transaction channels [>step-3]
		final List<String> channels = List.of(
			"unconfirmedAdded/" + monitorAddress,
			"unconfirmedRemoved/" + monitorAddress,
			"confirmedAdded/" + monitorAddress);
		for (final String channel : channels) {
			remote.sendText(JSON_MAPPER.createObjectNode()
				.put("uid", uid).put("subscribe", channel)
				.toString());
			System.out.printf(
				"Subscribed to %s channel%n", channel.split("/")[0]);
		} // [<step-3]

		// Build and announce a transfer transaction [>step-4]
		final JsonNode feeJSON = getJson("/network/fees/transaction");
		final long feeMultiplier = Math.max(
			feeJSON.get("medianFeeMultiplier").asLong(),
			feeJSON.get("minFeeMultiplier").asLong());

		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					new Address(monitorAddress),
					null,
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);

		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, transaction);
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(transaction, signature);
		transactionHash = facade.hashTransaction(transaction).toString();
		// [<step-4]
		announceTransaction(
			jsonPayload, "/transactions",
			"Announced transaction "
			+ transactionHash.substring(0, 16) + "...");

		// Wait for confirmation via WebSocket
		confirmed.join();
		// Unsubscribe before closing [>step-6]
		for (final String channel : channels)
			remote.sendText(JSON_MAPPER.createObjectNode()
				.put("uid", uid).put("unsubscribe", channel)
				.toString());
		System.out.println("Unsubscribed from all channels");
		session.close(); // [<step-6]
	}

	private JsonNode getJson(final String path)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body());
	}

	private void announceTransaction(
		final String payload,
		final String endpoint,
		final String label
	)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + endpoint))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		HTTP_CLIENT.send(request, BodyHandlers.ofString());
		System.out.println(label);
	}

	// Handle incoming messages [>step-5]
	@OnMessage
	public void onMessage(final String payload) throws IOException {
		final JsonNode message = JSON_MAPPER.readTree(payload);

		// Special case for the initial handshake message
		if (message.has("uid")) {
			uidFuture.complete(message.get("uid").asText());
			return;
		}

		final String topic = message.get("topic").asText();
		final String messageHash = message.get("data")
			.get("meta").get("hash").asText();
		final String name = topic.split("/")[0];
		System.out.printf("%s: hash=%s...%n",
			name, messageHash.substring(0, 16));

		if ("confirmedAdded".equals(name)
			&& messageHash.equals(transactionHash)) {
			System.out.printf("Transaction %s... confirmed%n",
				transactionHash.substring(0, 16));
			confirmed.complete(null);
		}
	} // [<step-5]
}
