//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.math.BigInteger;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.HexFormat;
import java.util.Locale;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.Network;

final class QueryBlockRewards {
	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final String NODE_URL = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private static final String BLOCK_HEIGHT =
		System.getenv().getOrDefault("BLOCK_HEIGHT", "3222290");

	private static JsonNode fetchJson(
		final String path
	) throws IOException, InterruptedException {
		final String url = String.format("%s%s", NODE_URL, path);
		final HttpRequest request =
			HttpRequest.newBuilder(URI.create(url)).GET().build();
		final HttpResponse<String> response =
			HTTP_CLIENT.send(request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body());
	}

	// Format an atomic amount as whole XYM with integer math,
	// since amounts can exceed float precision.
	private static String fmt(final BigInteger value) {
		final BigInteger divisor = BigInteger.valueOf(1_000_000);
		final BigInteger[] parts = value.divideAndRemainder(divisor);
		return String.format(Locale.US, "%,d.%06d", parts[0], parts[1]);
	}

	public static void main(final String[] args) {
		new QueryBlockRewards().run();
	}

	private void run() {
		System.out.printf("Using node %s%n", NODE_URL);

		try {
			// Get the block header [>step-1]
			final String blockPath = String.format(
				"/blocks/%s", BLOCK_HEIGHT);
			final JsonNode block = fetchJson(blockPath);
			final JsonNode blockHeader = block.get("block");
			final CryptoTypes.PublicKey signerPublicKey =
				new CryptoTypes.PublicKey(
					blockHeader.get("signerPublicKey").asText());
			final Address signer =
				Network.TESTNET.publicKeyToAddress(signerPublicKey);
			final String beneficiary =
				blockHeader.get("beneficiaryAddress").asText();
			System.out.printf("Block height: %s%n", BLOCK_HEIGHT);
			System.out.printf("Signer: %s%n", signer);
			final Address beneficiaryB32 =
				Address.fromDecodedAddressHexString(beneficiary);
			System.out.printf("Beneficiary: %s%n", beneficiaryB32);
			// [<step-1]
			// Get the network sink address [>step-2]
			final JsonNode properties = fetchJson("/network/properties");
			final String sinkB32 = properties.get("chain")
				.get("harvestNetworkFeeSinkAddress").asText();
			final String sink = HexFormat.of()
				.formatHex(new Address(sinkB32).bytes())
				.toUpperCase(Locale.ROOT);
			System.out.printf("Network sink: %s%n", sinkB32);
			// [<step-2]
			// Get the inflation reward at this height [>step-3]
			final String inflationPath = String.format(
				"/network/inflation/at/%s", BLOCK_HEIGHT);
			final JsonNode inflation = fetchJson(inflationPath);
			final BigInteger reward =
				new BigInteger(inflation.get("rewardAmount").asText());
			System.out.printf("Inflation reward: %s XYM%n", fmt(reward));
			// [<step-3]
			// Get harvest fee receipts for this block [>step-4]
			final String receiptsPath = String.format(
				"/statements/transaction?height=%s&receiptType=8515",
				BLOCK_HEIGHT);
			final JsonNode receipts = fetchJson(receiptsPath);

			// Label and display the reward distribution
			BigInteger total = BigInteger.ZERO;
			System.out.println("\nReward distribution:");
			for (final JsonNode item : receipts.get("data")) {
				for (final JsonNode receipt : item.get("statement")
					.get("receipts")) {
					if (8515 != receipt.get("type").asInt())
						continue;

					final BigInteger amount =
						new BigInteger(receipt.get("amount").asText());
					total = total.add(amount);
					final String target =
						receipt.get("targetAddress").asText();
					final String label;
					if (target.equals(sink)) {
						label = "Network sink (5%)";
					} else if (target.equals(beneficiary)) {
						label = "Beneficiary (25%)";
					} else {
						label = "Harvester";
						final Address harvesterAddress =
							Address.fromDecodedAddressHexString(target);
						System.out.printf(
							"  Harvester address: %s%n",
							harvesterAddress);
					}
					System.out.printf("  %s: %s XYM%n", label,
						fmt(amount));
				}
			}
			// [<step-4]
			// Summary [>step-5]
			final BigInteger fees = total.subtract(reward);
			System.out.println("\nSummary:");
			System.out.printf("  Total block reward: %s XYM%n",
				fmt(total));
			System.out.printf("  Inflation: %s XYM%n", fmt(reward));
			System.out.printf("  Transaction fees: %s XYM%n",
				fmt(fees)); // [<step-5]
		} catch (final Exception ex) {
			System.out.println(ex.getMessage());
		}
	}
}
