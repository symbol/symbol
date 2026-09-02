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

// Configuration
public final class MonitoringStatus {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private static final String NODE_URL = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	public static void main(final String[] args) {
		try {
			new MonitoringStatus().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", NODE_URL);

		// Transaction hash to monitor.
		final String transactionHash = // [>step-1]
			System.getenv().getOrDefault("TRANSACTION_HASH",
			"2B6D3B5232E06B9D32682F518C765301FCF9716BFA1EEEF95236534"
				+ "06E04C7EA");
		// [<step-1]
		System.out.printf("Monitoring transaction: %s%n", transactionHash);

		// Monitor the transaction until it's confirmed
		waitForTransactionConfirmation(transactionHash, 60, 2);
	}

	// [>step-2]
	/**
	 * Poll the transaction status endpoint until it is confirmed.
	 * @param txHash The hash of the transaction to monitor
	 * @param maxAttempts Maximum number of polling attempts
	 *   for confirmation
	 * @param waitSeconds Seconds to wait between attempts
	 * @return True if transaction was confirmed
	 */
	private boolean waitForTransactionConfirmation(
		final String txHash,
		final int maxAttempts,
		final int waitSeconds
	) throws IOException, InterruptedException {
		final String statusPath = "/transactionStatus/" + txHash;
		System.out.println("\nWaiting for transaction confirmation");
		System.out.printf("Polling %s%n", statusPath);

		for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
			// Query the transaction status endpoint
			final HttpRequest statusRequest = HttpRequest.newBuilder(
				URI.create(NODE_URL + statusPath)).GET().build();
			final HttpResponse<String> statusResponse = HTTP_CLIENT.send(
				statusRequest, BodyHandlers.ofString());

			if (404 == statusResponse.statusCode()) { // [>step-5]
				System.out.printf(
					"  Attempt %d: Transaction status not yet available%n",
					attempt);
			} // [<step-5]
			else {
				if (statusResponse.statusCode() / 100 != 2)
					throw new IOException(
						"HTTP " + statusResponse.statusCode());

				final JsonNode statusJSON = JSON_MAPPER.readTree(
					statusResponse.body());

				// Parse the response
				final String statusGroup =
					statusJSON.get("group").asText();
				final String statusCode = statusJSON.get("code").asText();
				final String statusHash = statusJSON.get("hash").asText();
				final String statusDeadline =
					statusJSON.get("deadline").asText();

				System.out.printf("  Attempt %d:%n", attempt);
				System.out.printf("    Status: %s%n", statusGroup);
				System.out.printf("    Code: %s%n", statusCode);
				System.out.printf("    Hash: %s%n", statusHash);
				System.out.printf("    Deadline: %s%n", statusDeadline);
				// [<step-2]
				// Check if the transaction has been confirmed [>step-3]
				if ("confirmed".equals(statusGroup)) {
					System.out.println("\nTransaction confirmed!");
					return true;
				} // [<step-3]

				// Check if the transaction failed [>step-4]
				if ("failed".equals(statusGroup)) {
					System.out.printf(
						"%nTransaction failed with code: %s%n",
						statusCode);
					throw new IOException(
						"Transaction failed: " + statusCode);
				} // [<step-4]
			}

			// Wait before next attempt (except on last attempt) [>step-6]
			if (attempt < maxAttempts)
				Thread.sleep(waitSeconds * 1000L);
			// [<step-6]
		}

		System.out.printf( // [>step-7]
			"%nTransaction not confirmed after %d attempts%n",
			maxAttempts);
		throw new IOException(
			"Transaction " + txHash + " not confirmed in time"
		); // [<step-7]
	}
}
