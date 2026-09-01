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
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

@ClientEndpoint
public final class ListenBondedTransactionFlow {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private CompletableFuture<String> uidFuture;

	private final CompletableFuture<Void> hashLockConfirmed =
		new CompletableFuture<>();

	private final CompletableFuture<Void> confirmed =
		new CompletableFuture<>();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final String wsUrl = nodeUrl.replaceFirst("http", "ws")
		+ "/ws";

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private KeyPair accountBKeyPair;

	private String bondedHash;

	private String hashLockHash;

	public static void main(final String[] args) {
		try {
			new ListenBondedTransactionFlow().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws Exception {
		System.out.printf("Using node %s%n", nodeUrl);
		// [>step-1]
		final String accountAPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_A_PRIVATE_KEY", "0".repeat(64));
		final String accountBPrivateKey = System.getenv().getOrDefault(
			"ACCOUNT_B_PRIVATE_KEY", "1".repeat(64));

		final KeyPair accountAKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountAPrivateKey));
		accountBKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(accountBPrivateKey));
		final Address accountAAddress = facade.network.publicKeyToAddress(
			accountAKeyPair.getPublicKey());
		final Address accountBAddress = facade.network.publicKeyToAddress(
			accountBKeyPair.getPublicKey());
		System.out.printf("Account A: %s%n", accountAAddress);
		System.out.printf("Account B: %s%n", accountBAddress);
		// [<step-1]

		// Fetch recommended fees
		final JsonNode feeJSON = getJson("/network/fees/transaction");
		final long feeMultiplier = Math.max(
			feeJSON.get("medianFeeMultiplier").asLong(),
			feeJSON.get("minFeeMultiplier").asLong());

		// [Account A] Build embedded transactions for the swap [>step-2]
		final EmbeddedTransaction embeddedTx1 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					accountBAddress,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
						new Amount(10_000_000))),
					null),
				accountAKeyPair.getPublicKey());

		final long customMosaicId = 0x6D1314BE751B62C2L;
		final EmbeddedTransaction embeddedTx2 =
			facade.createEmbeddedTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					accountAAddress,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(customMosaicId),
						new Amount(1))),
					null),
				accountBKeyPair.getPublicKey());

		// Build the bonded aggregate transaction
		final List<EmbeddedTransaction> embeddedTxs =
			List.of(embeddedTx1, embeddedTx2);
		final Transaction bondedTx =
			facade.createTransactionFromTypedDescriptor(
				new AggregateBondedTransactionV3Descriptor(
					SymbolFacade.hashEmbeddedTransactions(embeddedTxs),
					embeddedTxs,
					null),
				accountAKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60,
				1);

		// Sign the bonded aggregate
		final CryptoTypes.Signature bondedSignature =
			facade.signTransaction(accountAKeyPair, bondedTx);
		final String bondedPayload = SymbolTransactionFactory
			.attachSignature(bondedTx, bondedSignature);
		bondedHash = facade.hashTransaction(bondedTx).toString();
		System.out.println("[Account A] Bonded aggregate hash: "
			+ bondedHash.substring(0, 16) + "...");

		// Create the hash lock transaction
		final Transaction hashLock =
			facade.createTransactionFromTypedDescriptor(
				new HashLockTransactionV1Descriptor(
					new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
						new Amount(10_000_000)),
					new BlockDuration(100),
					new CryptoTypes.Hash256(bondedHash)),
				accountAKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		final CryptoTypes.Signature hashLockSignature =
			facade.signTransaction(accountAKeyPair, hashLock);
		final String hashLockPayload = SymbolTransactionFactory
			.attachSignature(hashLock, hashLockSignature);
		hashLockHash = facade.hashTransaction(hashLock).toString();

		// Confirm hash lock via WebSocket
		uidFuture = new CompletableFuture<>();
		final WebSocketContainer container =
			ContainerProvider.getWebSocketContainer();
		final Session lockSession = container.connectToServer(
			this, URI.create(wsUrl));
		final RemoteEndpoint.Basic lockRemote =
			lockSession.getBasicRemote();
		final String lockUid = uidFuture.join();

		final String addressA = accountAAddress.toString();
		final List<String> lockChannels = List.of(
			"confirmedAdded/" + addressA,
			"status/" + addressA);
		for (final String channel : lockChannels)
			lockRemote.sendText(JSON_MAPPER.createObjectNode()
				.put("uid", lockUid).put("subscribe", channel)
				.toString());

		// Announce hash lock
		announce("/transactions", hashLockPayload);
		System.out.println("[Account A] Announced hash lock "
			+ hashLockHash.substring(0, 16) + "...");

		// Wait for hash lock confirmation
		hashLockConfirmed.join();

		for (final String channel : lockChannels)
			lockRemote.sendText(JSON_MAPPER.createObjectNode()
				.put("uid", lockUid).put("unsubscribe", channel)
				.toString());
		lockSession.close();
		// [<step-2]
		// [Account B] Connect to WebSocket for bonded flow [>step-3]
		uidFuture = new CompletableFuture<>();
		final Session session = container.connectToServer(
			this, URI.create(wsUrl));
		final RemoteEndpoint.Basic remote = session.getBasicRemote();
		final String uid = uidFuture.join();
		System.out.printf("[Account B] Connected to %s with uid %s%n",
			wsUrl, uid);

		// Subscribe to bonded transaction channels
		final String addressB = accountBAddress.toString();
		final List<String> channels = List.of(
			"partialAdded/" + addressB,
			"partialRemoved/" + addressB,
			"cosignature/" + addressB,
			"unconfirmedAdded/" + addressB,
			"unconfirmedRemoved/" + addressB,
			"confirmedAdded/" + addressB,
			"status/" + addressB);
		for (final String channel : channels) {
			remote.sendText(JSON_MAPPER.createObjectNode()
				.put("uid", uid).put("subscribe", channel)
				.toString());
			System.out.printf(
				"[Account B] Subscribed to %s channel%n",
				channel.split("/")[0]);
		}
		// [<step-3]
		// [Account A] Announce bonded aggregate [>step-4]
		announce("/transactions/partial", bondedPayload);
		System.out.println("[Account A] Announced bonded "
			+ bondedHash.substring(0, 16) + "...");
		// [<step-4]

		// Wait for confirmation via WebSocket
		confirmed.join();
		// Unsubscribe before closing [>step-6]
		for (final String channel : channels)
			remote.sendText(JSON_MAPPER.createObjectNode()
				.put("uid", uid).put("unsubscribe", channel)
				.toString());
		System.out.println("[Account B] Unsubscribed from all channels");
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

	private void announce(final String path, final String payload)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		HTTP_CLIENT.send(request, BodyHandlers.ofString());
	}

	// [Account B] Listen for bonded transaction flow [>step-5]
	@OnMessage
	public void onMessage(final String payload) throws Exception {
		final JsonNode message = JSON_MAPPER.readTree(payload);

		// Special case for the initial handshake message
		if (message.has("uid")) {
			uidFuture.complete(message.get("uid").asText());
			return;
		}

		final String topic = message.get("topic").asText();
		final String name = topic.split("/")[0];

		if ("cosignature".equals(name)) {
			final String signer = message.get("data")
				.get("signerPublicKey").asText();
			System.out.println("cosignature: signer="
				+ signer.substring(0, 16) + "...");
		} else if ("status".equals(name)) {
			handleStatus(message);
		} else if ("partialAdded".equals(name)) {
			handlePartialAdded(message);
		} else if ("confirmedAdded".equals(name)) {
			handleConfirmedAdded(message);
		} else {
			final String messageHash = message.get("data")
				.get("meta").get("hash").asText();
			System.out.printf("%s: hash=%s...%n",
				name, messageHash.substring(0, 16));
		}
	}

	private void handleStatus(final JsonNode message) {
		final String statusHash = message.get("data").get("hash").asText();
		System.out.println("status: hash="
			+ statusHash.substring(0, 16) + "...");
		if (statusHash.equals(hashLockHash))
			hashLockConfirmed.completeExceptionally(new IOException(
				"Hash lock failed: "
				+ message.get("data").get("code").asText()));
		if (statusHash.equals(bondedHash))
			confirmed.completeExceptionally(new IOException(
				"Transaction failed: "
				+ message.get("data").get("code").asText()));
	}

	private void handlePartialAdded(final JsonNode message)
		throws IOException, InterruptedException {
		final String messageHash = message.get("data")
			.get("meta").get("hash").asText();
		System.out.println("partialAdded: hash="
			+ messageHash.substring(0, 16) + "...");
		if (messageHash.equals(bondedHash)) {
			final DetachedCosignature cosignature =
				SymbolFacade.cosignTransactionHashDetached(
					accountBKeyPair,
					new CryptoTypes.Hash256(bondedHash));
			final String cosignaturePayload = JSON_MAPPER
				.writeValueAsString(cosignature.toJson());
			announce("/transactions/cosignature", cosignaturePayload);
			System.out.println("[Account B] Submitted cosignature");
		}
	}

	private void handleConfirmedAdded(final JsonNode message) {
		final String messageHash = message.get("data")
			.get("meta").get("hash").asText();
		System.out.println("confirmedAdded: hash="
			+ messageHash.substring(0, 16) + "...");
		if (messageHash.equals(hashLockHash)) {
			System.out.println("Hash lock confirmed");
			hashLockConfirmed.complete(null);
		}
		if (messageHash.equals(bondedHash)) {
			System.out.println("Transaction "
				+ bondedHash.substring(0, 16) + "... confirmed");
			confirmed.complete(null);
		}
	}
	// [<step-5]
}
