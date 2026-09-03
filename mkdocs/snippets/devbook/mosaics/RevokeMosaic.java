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
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;

public final class RevokeMosaic {
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

	private JsonNode getAccountMosaics(
		final Address address
	) throws IOException, InterruptedException {
		final String accountPath = "/accounts/" + address;
		System.out.printf("Fetching account information from %s%n",
			accountPath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + accountPath)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body())
			.get("account").get("mosaics");
	}

	public static void main(final String[] args) {
		try {
			new RevokeMosaic().run();
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

		final String sourceAddressString = System.getenv().getOrDefault(
			"SOURCE_ADDRESS", "TB6QOVCUOFRCF5QJSKPIQMLUVWGJS3KYFDETRPA");
		final Address sourceAddress = new Address(sourceAddressString);
		System.out.printf("Source address: %s%n", sourceAddress);

		final String mosaicIdHex = System.getenv().getOrDefault(
			"MOSAIC_ID", "7AED3D514C986941");
		final long mosaicId = Long.parseUnsignedLong(mosaicIdHex, 16);
		System.out.printf("Mosaic ID: %d (0x%016X)%n",
			mosaicId, mosaicId);
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
		// --- CHECKING INITIAL BALANCE ---
		System.out.println("\n--- Checking initial balance ---");

		JsonNode mosaics = getAccountMosaics(sourceAddress); // [>step-3]
		for (final JsonNode mosaic : mosaics) {
			if (mosaic.get("id").asText().equals(
					mosaicIdHex.toUpperCase()))
				System.out.printf("  Mosaic ID: %s, Amount: %s%n",
					mosaic.get("id").asText(),
					mosaic.get("amount").asText());
		}
		// [<step-3]
		// --- REVOKING MOSAIC ---
		System.out.println("\n--- Revoking mosaic ---");

		final Transaction revokeTx = // [>step-4]
			facade.createTransactionFromTypedDescriptor(
				new MosaicSupplyRevocationTransactionV1Descriptor(
					sourceAddress,
					new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(mosaicId),
						new Amount(7_00))),
				signerKeyPair.getPublicKey(),
				feeMultiplier,
				2 * 60 * 60);
		// [<step-4]
		// Sign and generate final payload [>step-5]
		final CryptoTypes.Signature signature = facade.signTransaction(
			signerKeyPair, revokeTx);
		final String jsonPayload = SymbolTransactionFactory
			.attachSignature(revokeTx, signature);
		System.out.println("Built mosaic revocation transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(revokeTx.toJson()));

		final String revokeHash =
			facade.hashTransaction(revokeTx).toString();
		System.out.printf("Transaction hash: %s%n", revokeHash);

		// Announce transaction
		announceTransaction(jsonPayload, "mosaic revocation");
		// [<step-5]
		// Wait for confirmation [>step-6]
		waitForConfirmation(revokeHash, "mosaic revocation");
		// [<step-6]
		// --- VERIFYING REVOCATION ---
		System.out.println("\n--- Verifying revocation ---");

		mosaics = getAccountMosaics(sourceAddress); // [>step-7]
		for (final JsonNode mosaic : mosaics) {
			if (mosaic.get("id").asText().equals(
					mosaicIdHex.toUpperCase()))
				System.out.printf("  Mosaic ID: %s, Amount: %s%n",
					mosaic.get("id").asText(),
					mosaic.get("amount").asText());
		}
		// [<step-7]
	}
}
