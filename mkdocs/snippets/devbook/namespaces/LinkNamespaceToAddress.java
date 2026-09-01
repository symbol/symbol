//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.List;

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

public final class LinkNamespaceToAddress {
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
			try {
				final JsonNode status = getJson(
					"/transactionStatus/" + transactionHash);
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
			} catch (final IOException ex) {
				if (ex.getMessage().contains("failed"))
					throw ex;

				System.out.println("  Transaction status: unknown");
			}
		}
		throw new IOException(String.format(
			"%s not confirmed after 60 seconds", label));
	}

	public static void main(final String[] args) {
		try {
			new LinkNamespaceToAddress().run();
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
		// [>step-2]
		final String namespaceName = System.getenv().getOrDefault(
			"NAMESPACE_NAME", "my_namespace");
		System.out.printf("Namespace name: %s%n", namespaceName);

		final List<Long> nsPath = IdGenerator.generateNamespacePath(
			namespaceName);
		final long namespaceId = nsPath.get(nsPath.size() - 1);
		System.out.printf("Namespace ID: %s (0x%016X)%n",
			Long.toUnsignedString(namespaceId), namespaceId);

		// Target address to link the namespace to
		final Address targetAddress = new Address(
			System.getenv().getOrDefault(
				"TARGET_ADDRESS",
				"TCWYXKVYBMO4NBCUF3AXKJMXCGVSYQOS7ZG2TLI"));
		System.out.printf("Target address: %s%n", targetAddress);
		// [<step-2]
		// Fetch recommended fees [>step-3]
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final JsonNode feeJson = getJson(feePath);
		final long feeMultiplier = Math.max(
			feeJson.get("medianFeeMultiplier").asLong(),
			feeJson.get("minFeeMultiplier").asLong());
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		// [<step-3]
		// Build the alias transaction [>step-4]
		final Transaction transaction =
			facade.createTransactionFromTypedDescriptor(
				new AddressAliasTransactionV1Descriptor(
					new NamespaceId(namespaceId),
					targetAddress,
					AliasAction.LINK),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-4]
		// Sign transaction and generate final payload [>step-5]
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(transaction,
				facade.signTransaction(signerKeyPair, transaction));
		System.out.println("Address alias transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(transaction.toJson()));

		final String transactionHash =
			facade.hashTransaction(transaction).toString();
		System.out.printf("Transaction hash: %s%n", transactionHash);

		// Announce and confirm transaction
		announceTransaction(jsonPayload, "address alias transaction");
		waitForConfirmation(transactionHash, "address alias transaction");
		// [<step-5]
		// Retrieve the namespace to verify the alias [>step-6]
		final String namespacePath = "/namespaces/%016X".formatted(
			namespaceId);
		System.out.printf("Fetching namespace information from %s%n",
			namespacePath);
		final JsonNode namespaceInfo = getJson(namespacePath)
			.get("namespace");
		System.out.println("Alias information:");
		final int aliasType = namespaceInfo.get("alias").get("type")
			.asInt();
		System.out.printf("  Alias type: %d%n", aliasType);
		if (2 == aliasType) { // ADDRESS type
			final Address aliasedAddress = Address
				.fromDecodedAddressHexString(
					namespaceInfo.get("alias").get("address").asText());
			System.out.printf("  Linked address: %s%n",
				aliasedAddress);
		}
		// [<step-6]
		// [>step-7]
		// Send a transfer using the alias instead of a raw address
		System.out.printf("Using alias in transfer: %s%n",
			namespaceName);

		// Encode the namespace ID as a recipient address
		final long recipientId = IdGenerator.generateNamespacePath(
			namespaceName).get(nsPath.size() - 1);
		final Address recipientAddress = Address.fromNamespaceId(
			new NamespaceId(recipientId), facade.network.identifier);
		System.out.printf("Recipient address (alias): %s%n",
			recipientAddress);

		final Transaction testTransaction =
			facade.createTransactionFromTypedDescriptor(
				new TransferTransactionV1Descriptor(
					recipientAddress,
					List.of(new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
						new Amount(1_000_000))), // 1 XYM
					null),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		final String testJsonPayload = SymbolTransactionFactory
			.attachSignature(testTransaction,
				facade.signTransaction(signerKeyPair,
					testTransaction));
		System.out.println("Test transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(testTransaction.toJson()));
		final String testTransactionHash =
			facade.hashTransaction(testTransaction).toString();
		System.out.printf("Transaction hash: %s%n", testTransactionHash);
		announceTransaction(testJsonPayload, "test transaction");
		waitForConfirmation(testTransactionHash, "test transaction");
		// [<step-7]
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
