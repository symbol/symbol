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

public final class RegisterSubnamespace {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private void announceTransaction(
		final String payload,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Announcing %s to /transactions%n", label);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + "/transactions"))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", response.body());
	}

	private void waitForConfirmation(
		final String transactionHash,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Waiting for %s confirmation...%n", label);
		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			final String statusPath =
				"/transactionStatus/" + transactionHash;
			final HttpRequest statusRequest = HttpRequest.newBuilder(
				URI.create(nodeUrl + statusPath)).GET().build();
			final HttpResponse<String> statusResponse = HTTP_CLIENT
				.send(statusRequest, BodyHandlers.ofString());
			if (404 == statusResponse.statusCode()) {
				System.out.println("  Transaction status: unknown");
				continue;
			}
			if (2 != statusResponse.statusCode() / 100)
				throw new IOException(
					"HTTP " + statusResponse.statusCode());

			final JsonNode status =
				JSON_MAPPER.readTree(statusResponse.body());
			final String group = status.get("group").asText();
			System.out.printf("  Transaction status: %s%n", group);
			if ("confirmed".equals(group)) {
				System.out.printf("%s confirmed in %d seconds%n",
					label, attempt);
				return;
			}
			if ("failed".equals(group))
				throw new IOException(String.format("%s failed: %s",
					label, status.get("code").asText()));
		}
		throw new IOException(String.format(
			"%s not confirmed after 60 seconds", label));
	}

	public static void main(final String[] args) {
		try {
			new RegisterSubnamespace().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		final String signerPrivateKey = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(signerPrivateKey));

		final Address signerAddress = facade.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.printf("Signer address: %s%n", signerAddress);

		// Fetch recommended fees
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final JsonNode feeJson = getJson(feePath);
		final long feeMultiplier = Math.max(
			feeJson.get("medianFeeMultiplier").asLong(),
			feeJson.get("minFeeMultiplier").asLong());
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);

		// Build the subnamespace name [>step-1]
		final String rootNamespaceName = System.getenv().getOrDefault(
			"ROOT_NAMESPACE", "ns_root");
		final String subnamespaceName = System.getenv().getOrDefault(
			"SUBNAMESPACE", "sub_" + System.currentTimeMillis());
		final String fullNamespaceName =
			rootNamespaceName + "." + subnamespaceName;
		System.out.printf("Creating subnamespace: %s%n",
			fullNamespaceName);

		// Generate the parent namespace ID from the root name
		final long parentId = IdGenerator.generateNamespaceId(
			rootNamespaceName);
		System.out.printf("Parent namespace ID: 0x%016X%n", parentId);
		// [<step-1]
		// Build the transaction [>step-2]
		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new NamespaceRegistrationTransactionV1Descriptor(
					new NamespaceId(0),
					NamespaceRegistrationType.CHILD,
					null,
					new NamespaceId(parentId),
					subnamespaceName.getBytes(StandardCharsets.UTF_8)),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-2]
		// Sign transaction and generate final payload
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
		announceTransaction(jsonPayload, "namespace registration");

		// Wait for confirmation
		waitForConfirmation(transactionHash, "namespace registration");

		// Retrieve the namespace [>step-3]
		final long namespaceId = IdGenerator.generateNamespaceId(
			subnamespaceName, parentId);
		System.out.printf("Child namespace ID: %s (0x%016X)%n",
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
		System.out.printf("  Parent ID: %s%n",
			namespaceInfo.get("parentId").asText());
		System.out.printf("  Depth: %s%n",
			namespaceInfo.get("depth").asText());
		System.out.printf("  Level 0: %s%n",
			namespaceInfo.get("level0").asText());
		if (1 <= namespaceInfo.get("depth").asInt())
			System.out.printf("  Level 1: %s%n",
				namespaceInfo.get("level1").asText());
		if (2 <= namespaceInfo.get("depth").asInt()
			&& namespaceInfo.has("level2"))
			System.out.printf("  Level 2: %s%n",
				namespaceInfo.get("level2").asText());
		System.out.printf("  Start height: %s%n",
			namespaceInfo.get("startHeight").asText());
		System.out.printf("  End height: %s%n",
			namespaceInfo.get("endHeight").asText()); // [<step-3]
	}

	private JsonNode getJson(final String path)
		throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body());
	}

}
