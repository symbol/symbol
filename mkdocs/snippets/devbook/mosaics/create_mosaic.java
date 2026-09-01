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

final class CreateMosaic {
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
				final String statusPath =
					"/transactionStatus/" + transactionHash;
				final HttpRequest statusRequest = HttpRequest.newBuilder(
					URI.create(nodeUrl + statusPath)).GET().build();
				final HttpResponse<String> statusResponse = HTTP_CLIENT
					.send(statusRequest, BodyHandlers.ofString());
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
			new CreateMosaic().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
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
		System.out.println("\n--- Creating mosaic definition ---");

		// [>step-3]
		final long nonce = System.currentTimeMillis() & 0xFFFFFFFFL;
		System.out.printf("Mosaic nonce: %d%n", nonce);
		// [<step-3]
		// Build the mosaic definition transaction [>step-4]
		final Transaction definitionTx =
			facade.createTransactionFromTypedDescriptor(
				new MosaicDefinitionTransactionV1Descriptor(
					new MosaicId(0),
					new BlockDuration(0),
					new MosaicNonce(nonce),
					new MosaicFlags(
						MosaicFlags.TRANSFERABLE.value |
						MosaicFlags.RESTRICTABLE.value),
					2),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);

		final long mosaicId = IdGenerator.generateMosaicId(
			signerAddress, nonce);
		System.out.printf("Mosaic ID: %d (0x%016X)%n",
			mosaicId, mosaicId);
		// [<step-4]
		// Sign and generate final payload [>step-5]
		final CryptoTypes.Signature defSignature = facade.signTransaction(
			signerKeyPair, definitionTx);
		final String defPayload = SymbolTransactionFactory.attachSignature(
			definitionTx, defSignature);
		System.out.println("Built mosaic definition transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(definitionTx.toJson()));

		// Announce and wait for confirmation
		final String definitionHash =
			facade.hashTransaction(definitionTx).toString();
		System.out.printf("Transaction hash: %s%n", definitionHash);
		announceTransaction(defPayload, "mosaic definition");
		waitForConfirmation(definitionHash, "mosaic definition");
		// [<step-5]
		System.out.println("\n--- Increasing mosaic supply ---");

		final Transaction supplyTx = // [>step-6]
			facade.createTransactionFromTypedDescriptor(
				new MosaicSupplyChangeTransactionV1Descriptor(
					new UnresolvedMosaicId(mosaicId),
					new Amount(100_00),
					MosaicSupplyChangeAction.INCREASE),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-6]
		// Sign and generate final payload [>step-7]
		final CryptoTypes.Signature supSignature = facade.signTransaction(
			signerKeyPair, supplyTx);
		final String supPayload = SymbolTransactionFactory.attachSignature(
			supplyTx, supSignature);
		System.out.println("Built mosaic supply change transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(supplyTx.toJson()));

		// Announce and wait for confirmation
		final String supplyHash =
			facade.hashTransaction(supplyTx).toString();
		System.out.printf("Transaction hash: %s%n", supplyHash);
		announceTransaction(supPayload, "mosaic supply change");
		waitForConfirmation(supplyHash, "mosaic supply change");
		// [<step-7]
		System.out.println("\n--- Verifying mosaic ---");

		// [>step-8]
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
		System.out.printf("  Flags: %s%n",
			mosaicInfo.get("flags").asText());
		System.out.printf("  Divisibility: %s%n",
			mosaicInfo.get("divisibility").asText());
		System.out.printf("  Duration: %s%n",
			mosaicInfo.get("duration").asText());
		// [<step-8]
	}
}
