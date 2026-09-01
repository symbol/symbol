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

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

final class ModifyMosaicDefinition {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final SymbolFacade facade = new SymbolFacade("testnet");

	public static void main(final String[] args) {
		try {
			new ModifyMosaicDefinition().run();
		} catch (final Exception ex) {
			System.out.println(ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// [>step-1]
		final String privateKeyString = System.getenv().getOrDefault(
			"SIGNER_PRIVATE_KEY", "0".repeat(64));
		final KeyPair signerKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(privateKeyString));

		final Address signerAddress = facade.network.publicKeyToAddress(
			signerKeyPair.getPublicKey());
		System.out.printf("Signer address: %s%n", signerAddress);
		// [<step-1]

		// Fetch recommended fees [>step-2]
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final HttpRequest feeRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + feePath)).GET().build();
		final HttpResponse<String> feeResponse = HTTP_CLIENT.send(
			feeRequest, BodyHandlers.ofString());
		final JsonNode feeJson = JSON_MAPPER.readTree(feeResponse.body());
		final long feeMultiplier = Math.max(
			feeJson.get("medianFeeMultiplier").asLong(),
			feeJson.get("minFeeMultiplier").asLong());
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		// [<step-2]

		// Build the modification transaction [>step-3]
		final long mosaicNonce = Long.parseUnsignedLong(
			System.getenv().getOrDefault("MOSAIC_NONCE", "0"));
		System.out.printf("Mosaic nonce: %d%n", mosaicNonce);

		final long mosaicId = IdGenerator.generateMosaicId(
			signerAddress, mosaicNonce);
		System.out.printf("Mosaic ID: %d (0x%016X)%n",
			mosaicId, mosaicId);

		final Transaction modifyTx =
			facade.createTransactionFromTypedDescriptor(
				new MosaicDefinitionTransactionV1Descriptor(
					new MosaicId(0),
					new BlockDuration(0),
					new MosaicNonce(mosaicNonce),
					MosaicFlags.REVOKABLE,
					0),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-3]
		// Sign and generate final payload [>step-4]
		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, modifyTx);
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(modifyTx, signature);
		System.out.println("Built mosaic modification transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(modifyTx.toJson()));

		// Announce transaction
		final String modifyHash =
			facade.hashTransaction(modifyTx).toString();
		System.out.printf("Transaction hash: %s%n", modifyHash);

		System.out.println(
			"Announcing mosaic modification to /transactions");
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + "/transactions"))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(jsonPayload))
			.build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", response.body());
		// [<step-4]
		// Wait for confirmation [>step-5]
		System.out.println(
			"Waiting for mosaic modification confirmation...");
		for (int attempt = 0; 60 > attempt; ++attempt) {
			Thread.sleep(1000);
			try {
				final String statusPath =
					"/transactionStatus/" + modifyHash;
				final HttpRequest statusRequest = HttpRequest.newBuilder(
					URI.create(nodeUrl + statusPath)).GET().build();
				final HttpResponse<String> statusResponse = HTTP_CLIENT
					.send(statusRequest, BodyHandlers.ofString());
				final JsonNode status =
					JSON_MAPPER.readTree(statusResponse.body());
				final String group = status.get("group").asText();
				System.out.printf("  Transaction status: %s%n", group);
				if ("confirmed".equals(group)) {
					System.out.printf(
						"Mosaic modification confirmed in %d seconds%n",
						attempt);
					break;
				}
				if ("failed".equals(group))
					throw new IOException(String.format(
						"Mosaic modification failed: %s",
						status.get("code").asText()));
			} catch (final IOException ex) {
				if (ex.getMessage().contains("failed"))
					throw ex;

				System.out.println("  Transaction status: unknown");
			}
		}
		// [<step-5]
		// Retrieve the mosaic [>step-6]
		final String mosaicIdHex = "%016X".formatted(mosaicId);
		final String mosaicPath = "/mosaics/" + mosaicIdHex;
		System.out.printf("Fetching mosaic information from %s%n",
			mosaicPath);
		final HttpRequest mosaicRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + mosaicPath)).GET().build();
		final HttpResponse<String> mosaicResponse = HTTP_CLIENT.send(
			mosaicRequest, BodyHandlers.ofString());
		final JsonNode mosaicInfo = JSON_MAPPER.readTree(
			mosaicResponse.body()).get("mosaic");
		System.out.println("Mosaic information:");
		System.out.printf("  Mosaic ID: %s%n",
			mosaicInfo.get("id").asText());
		System.out.printf("  Supply: %s%n",
			mosaicInfo.get("supply").asText());
		System.out.printf("  Divisibility: %s%n",
			mosaicInfo.get("divisibility").asText());
		System.out.printf("  Flags: %s%n",
			mosaicInfo.get("flags").asText());
		System.out.printf("  Duration: %s%n",
			mosaicInfo.get("duration").asText());
		// [<step-6]
	}
}
