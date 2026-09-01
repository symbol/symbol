//JAVA 21+
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.Merkle;
import org.symbol.sdk.symbol.Merkle.BranchNode;
import org.symbol.sdk.symbol.Merkle.LeafNode;
import org.symbol.sdk.symbol.Merkle.PatriciaMerkleProofResult;
import org.symbol.sdk.symbol.Merkle.TreeNode;
import org.symbol.sdk.utils.Converter;
import org.symbol.sdk.utils.Transforms;

final class ProveMosaicDefinition {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	public static void main(final String[] args) {
		try {
			new ProveMosaicDefinition().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		// Fetch the network currency mosaic ID [>step-1]
		final JsonNode props = getJson("/network/properties");
		final String rawId = props.get("chain")
			.get("currencyMosaicId").asText();
		final long mosaicId = Long.parseUnsignedLong(
			rawId.substring(2).replace("'", ""), 16);
		final String mosaicIdHex = "%016X".formatted(mosaicId);
		System.out.printf("Currency mosaic ID: %s%n", mosaicIdHex);

		// Fetch the mosaic properties
		final String mosaicPath = "/mosaics/" + mosaicIdHex;
		System.out.printf("Fetching mosaic from %s%n", mosaicPath);
		final JsonNode mosaicData = getJson(mosaicPath);
		final JsonNode mosaic = mosaicData.get("mosaic");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(mosaic));
		// [<step-1]
		// Serialize and hash the mosaic properties [>step-2]
		final ByteBuffer buffer = ByteBuffer.allocate(40 + 24)
			.order(ByteOrder.LITTLE_ENDIAN);
		buffer.putShort((short) mosaic.get("version").asInt());
		buffer.putLong(Long.parseUnsignedLong(
			mosaic.get("id").asText(), 16));
		buffer.putLong(Long.parseUnsignedLong(
			mosaic.get("supply").asText()));
		buffer.putLong(Long.parseUnsignedLong(
			mosaic.get("startHeight").asText()));
		buffer.put(Converter.hexToUint8(
			mosaic.get("ownerAddress").asText()));
		buffer.putInt(mosaic.get("revision").asInt());
		buffer.put((byte) mosaic.get("flags").asInt());
		buffer.put((byte) mosaic.get("divisibility").asInt());
		buffer.putLong(Long.parseUnsignedLong(
			mosaic.get("duration").asText()));
		final CryptoTypes.Hash256 hashedValue =
			new CryptoTypes.Hash256(Transforms.sha3_256(
				buffer.array()));
		System.out.printf("Hashed value: %s%n", hashedValue);

		// Hash the mosaic ID to get the encoded key
		final ByteBuffer keyBuffer = ByteBuffer.allocate(8)
			.order(ByteOrder.LITTLE_ENDIAN);
		keyBuffer.putLong(Long.parseUnsignedLong(
			mosaic.get("id").asText(), 16));
		final CryptoTypes.Hash256 encodedKey =
			new CryptoTypes.Hash256(Transforms.sha3_256(
				keyBuffer.array()));
		System.out.printf("Encoded key: %s%n", encodedKey);
		// [<step-2]
		// Fetch the current network height [>step-3]
		final JsonNode chainInfo = getJson("/chain/info");
		final long height = chainInfo.get("height").asLong();
		System.out.printf("Current height: %d%n", height);

		// Fetch the block's state hash and roots
		final String blockPath = "/blocks/" + height;
		System.out.printf("Fetching block from %s%n", blockPath);
		final JsonNode blockData = getJson(blockPath);
		final CryptoTypes.Hash256 stateHash =
			new CryptoTypes.Hash256(blockData.get("block")
				.get("stateHash").asText());
		final List<CryptoTypes.Hash256> roots = new ArrayList<>();
		for (final JsonNode root : blockData.get("meta")
			.get("stateHashSubCacheMerkleRoots"))
			roots.add(new CryptoTypes.Hash256(root.asText()));
		System.out.printf("State hash: %s%n", stateHash);
		// [<step-3]
		// Fetch the patricia tree path [>step-4]
		final String treeUrl = "/mosaics/" + mosaicIdHex + "/merkle";
		System.out.printf("Fetching tree path from %s%n", treeUrl);
		final JsonNode treeData = getJson(treeUrl);
		final List<TreeNode> merklePath =
			Merkle.deserializePatriciaTreeNodes(
				Converter.hexToUint8(treeData.get("raw").asText()));
		System.out.printf("Tree path: %d nodes%n", merklePath.size());
		final String keyHex = encodedKey.toString();
		int keyPos = 0;
		for (int i = 0; merklePath.size() > i; ++i) {
			final TreeNode node = merklePath.get(i);
			keyPos += node.path.size();
			final String pathStr = 0 != node.path.size()
				? "  path: " + node.hexPath()
				: "";
			if (node instanceof LeafNode leaf) {
				System.out.printf("  [%d] leaf%s  value: %s%n",
					i, pathStr, leaf.value);
			} else if (node instanceof BranchNode branch) {
				final char nibble = keyHex.charAt(keyPos);
				++keyPos;
				final List<String> active = new ArrayList<>();
				for (int j = 0; branch.links.length > j; ++j) {
					if (null != branch.links[j])
						active.add("%X".formatted(j));
				}
				System.out.printf(
					"  [%d] branch%s  links: [%s]  -> follow %c%n",
					i, pathStr, String.join(",", active), nibble);
			}
		}
		// [<step-4]
		// Verify the mosaic state [>step-5]
		final int result = Merkle.provePatriciaMerkle(
			encodedKey, hashedValue, merklePath, stateHash, roots);

		if (PatriciaMerkleProofResult.VALID_POSITIVE == result) {
			System.out.printf(
				"Mosaic %s state verified at height %d%n",
				mosaicIdHex, height);
		} else {
			throw new IllegalStateException(String.format(
				"Mosaic %s proof failed: %d",
				mosaicIdHex, result));
		} // [<step-5]
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
