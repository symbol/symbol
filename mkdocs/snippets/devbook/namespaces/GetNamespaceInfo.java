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

import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;

public final class GetNamespaceInfo {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private final String nodeUrl = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	public static void main(final String[] args) {
		try {
			new GetNamespaceInfo().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", nodeUrl);

		final String namespaceName = System.getenv().getOrDefault(
			"NAMESPACE_NAME", "symbol.xym");
		System.out.printf("Namespace name: %s%n", namespaceName);

		// Generate namespace ID from name [>step-1]
		final List<Long> path = IdGenerator.generateNamespacePath(
			namespaceName);
		final long namespaceId = path.get(path.size() - 1);
		final String namespaceIdHex = "%016X".formatted(namespaceId);
		System.out.printf("Namespace ID: %s (0x%s)%n",
			Long.toUnsignedString(namespaceId), namespaceIdHex);
		// [<step-1]
		// Fetch namespace information [>step-2]
		final String namespacePath = "/namespaces/" + namespaceIdHex;
		System.out.printf("Fetching namespace information from %s%n",
			namespacePath);
		final HttpRequest namespaceRequest = HttpRequest.newBuilder(
			URI.create(nodeUrl + namespacePath)).GET().build();
		final HttpResponse<String> namespaceResponse = HTTP_CLIENT.send(
			namespaceRequest, BodyHandlers.ofString());
		final JsonNode ns = JSON_MAPPER.readTree(
			namespaceResponse.body()).get("namespace");
		System.out.println("Namespace information:");
		System.out.printf("  Registration type: %s%n",
			ns.get("registrationType").asText());
		final Address ownerAddress = Address
			.fromDecodedAddressHexString(ns.get("ownerAddress").asText());
		System.out.printf("  Owner address: %s%n", ownerAddress);
		final int depth = ns.get("depth").asInt();
		System.out.printf("  Depth: %d%n", depth);
		System.out.printf("  Level 0 ID: %s%n",
			ns.get("level0").asText());
		if (2 <= depth)
			System.out.printf("  Level 1 ID: %s%n",
				ns.get("level1").asText());
		if (3 == depth && ns.has("level2"))
			System.out.printf("  Level 2 ID: %s%n",
				ns.get("level2").asText());
		System.out.printf("  Start height: %s%n",
			ns.get("startHeight").asText());
		final long endHeight = Long.parseUnsignedLong(
			ns.get("endHeight").asText());
		System.out.printf("  End height: %s (0x%X)%n",
			Long.toUnsignedString(endHeight), endHeight);
		// [<step-2]
		// Display alias information [>step-3]
		final JsonNode alias = ns.get("alias");
		final int aliasType = alias.get("type").asInt();
		System.out.printf("  Alias type: %d%n", aliasType);
		if (1 == aliasType) {
			System.out.printf("  Linked mosaic ID: %s%n",
				alias.get("mosaicId").asText());
		} else if (2 == aliasType) {
			final Address linkedAddress = Address
				.fromDecodedAddressHexString(
					alias.get("address").asText());
			System.out.printf("  Linked address: %s%n", linkedAddress);
		} else {
			System.out.println("  No alias linked");
		} // [<step-3]
	}
}
