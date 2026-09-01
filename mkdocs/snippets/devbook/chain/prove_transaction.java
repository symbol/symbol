//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.ArrayList;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.Merkle;
import org.symbol.sdk.symbol.Merkle.MerklePart;

final class ProveTransaction {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private final String txHash = System.getenv().getOrDefault(
		"TRANSACTION_HASH",
		"99011A8DBC086E0C359E9D8A38FEC6714C33726FCD0C1B5C0F772A8240"
			+ "0D808B");

	public static void main(final String[] args) {
		try {
			new ProveTransaction().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);
		System.out.printf("Transaction hash: %s%n", txHash);

		// [>step-1]
		// Fetch the confirmed transaction to get its block height
		final String txPath = "/transactions/confirmed/" + txHash;
		System.out.printf("Fetching transaction from %s%n", txPath);
		final JsonNode txData = getJson(txPath);

		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(txData.get("meta")));
		final long blockHeight = txData.get("meta").get("height")
			.asLong();
		final CryptoTypes.Hash256 merkleComponentHash =
			new CryptoTypes.Hash256(txData.get("meta")
				.get("merkleComponentHash").asText());
		// [<step-1]
		// [>step-2]
		// Fetch the block header to get the transactions hash
		final String blockPath = "/blocks/" + blockHeight;
		System.out.printf("Fetching block from %s%n", blockPath);
		final JsonNode blockData = getJson(blockPath);

		final JsonNode blockSummary = JSON_MAPPER.createObjectNode()
			.put("height", blockData.get("block")
				.get("height").asText())
			.put("transactionsHash", blockData.get("block")
				.get("transactionsHash").asText());
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(blockSummary));
		final CryptoTypes.Hash256 transactionsHash =
			new CryptoTypes.Hash256(blockData.get("block")
				.get("transactionsHash").asText());
		// [<step-2]
		// Fetch the merkle proof path for the transaction [>step-3]
		final String merklePath = "/blocks/" + blockHeight
			+ "/transactions/" + txHash + "/merkle";
		System.out.println("Fetching merkle proof:");
		System.out.printf("  %s%n", merklePath);
		final JsonNode merkleData = getJson(merklePath);

		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(merkleData));

		// Convert the API response to the format expected by the SDK
		final List<MerklePart> merkleProofPath = new ArrayList<>();
		for (final JsonNode part : merkleData.get("merklePath"))
			merkleProofPath.add(new MerklePart(
				new CryptoTypes.Hash256(part.get("hash").asText()),
				"left".equals(part.get("position").asText())));
		System.out.printf("  Merkle path length: %d%n",
			merkleProofPath.size());
		// [<step-3]
		// [>step-4]
		// Verify that the transaction is included in the block
		final boolean isProven = Merkle.proveMerkle(
			merkleComponentHash, merkleProofPath, transactionsHash);

		if (isProven) {
			System.out.printf(
				"Transaction %s... proven in block %d%n",
				txHash.substring(0, 16), blockHeight);
		} else {
			throw new IllegalStateException(String.format(
				"Transaction %s... NOT proven in block %d",
				txHash.substring(0, 16), blockHeight));
		} // [<step-4]
	}

	private JsonNode getJson(
		final String path
	) throws IOException, InterruptedException {
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(nodeUrl + path)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		return JSON_MAPPER.readTree(response.body());
	}
}
