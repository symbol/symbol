//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.nio.charset.StandardCharsets;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class RegisterRootNamespace {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	public static void main(final String[] args) {
		try {
			new RegisterRootNamespace().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);
		// [>step-1]
		final String signerPrivateKey = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(signerPrivateKey));

		final Address signerAddress = facade.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.printf("Signer address: %s%n", signerAddress);
		// [<step-1]
		// Fetch recommended fees [>step-2]
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final JsonNode feeJson = getJson(feePath);
		final long feeMultiplier = Math.max(
			feeJson.get("medianFeeMultiplier").asLong(),
			feeJson.get("minFeeMultiplier").asLong());
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		// [<step-2]
		// Build the namespace name [>step-3]
		final String namespaceName = System.getenv().getOrDefault(
			"ROOT_NAMESPACE",
			"ns_" + System.currentTimeMillis() / 1000);
		System.out.printf("Creating root namespace: %s%n",
			namespaceName);
		// [<step-3]
		// Build the transaction [>step-4]
		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new NamespaceRegistrationTransactionV1Descriptor(
					new NamespaceId(0),
					NamespaceRegistrationType.ROOT,
					new BlockDuration(86400), // approximately 30 days
					null,
					namespaceName.getBytes(StandardCharsets.UTF_8)),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-4]
		// Sign transaction and generate final payload [>step-5]
		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, transaction);
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(transaction, signature);
		System.out.println("Built transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf("Transaction hash: %s%n", transactionHash);

		// Announce transaction
		System.out.println(
			"Announcing namespace registration to /transactions");
		final HttpRequest announceRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + "/transactions"))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(jsonPayload))
			.build();
		final HttpResponse<String> announceResponse = HTTP_CLIENT.send(
			announceRequest, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n",
			announceResponse.body());
		// [<step-5]
		// Wait for confirmation
		System.out.println(
			"Waiting for namespace registration confirmation...");
		waitForConfirmation(transactionHash);

		// Retrieve the namespace [>step-7]
		final long namespaceId = IdGenerator.generateNamespaceId(
			namespaceName);
		System.out.printf("Namespace ID: %s (0x%016X)%n",
			Long.toUnsignedString(namespaceId), namespaceId);

		final String namespacePath = "/namespaces/%016X".formatted(
			namespaceId);
		System.out.printf("Fetching namespace information from %s%n",
			namespacePath);
		final JsonNode namespaceInfo = getJson(namespacePath)
			.get("namespace");
		System.out.println("Namespace information:");
		System.out.printf("  Registration type: %s%n",
			namespaceInfo.get("registrationType").asText());
		final Address ownerAddress = Address
			.fromDecodedAddressHexString(
				namespaceInfo.get("ownerAddress").asText());
		System.out.printf("  Owner address: %s%n", ownerAddress);
		System.out.printf("  Start height: %s%n",
			namespaceInfo.get("startHeight").asText());
		System.out.printf("  End height: %s%n",
			namespaceInfo.get("endHeight").asText()); // [<step-7]
	}

	private JsonNode getJson(final String path)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body());
	}

	// [>step-6]
	private void waitForConfirmation(final String transactionHash)
		throws IOException, InterruptedException {
		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			try {
				final JsonNode status = getJson(
					"/transactionStatus/" + transactionHash);
				final String group = status.get("group").asText();
				System.out.printf("  Transaction status: %s%n", group);
				if ("confirmed".equals(group)) {
					System.out.printf(
						"Namespace registration confirmed in %d seconds%n",
						attempt);
					return;
				}
				if ("failed".equals(group))
					throw new IOException(
						"Namespace registration failed: "
						+ status.get("code").asText());
			} catch (final IOException ex) {
				if (ex.getMessage().contains("failed"))
					throw ex;

				System.out.println("  Transaction status: unknown");
			}
		}
	} // [<step-6]
}
