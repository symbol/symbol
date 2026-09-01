//JAVA 21+

import java.io.IOException;
import java.math.BigDecimal;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.util.Locale;

final class QueryCurrencySupply {
	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private static final String NODE_URL = System.getenv().getOrDefault(
		"NODE_URL", "https://reference.symboltest.net:3001");

	private static BigDecimal fetchSupplyValue(
		final String supplyType
	) throws IOException, InterruptedException {
		final String supplyPath =
			String.format("/network/currency/supply/%s", supplyType);
		final String url = String.format("%s%s", NODE_URL, supplyPath);
		final HttpRequest request =
			HttpRequest.newBuilder(URI.create(url)).GET().build();
		final HttpResponse<String> response =
			HTTP_CLIENT.send(request, BodyHandlers.ofString());
		return new BigDecimal(response.body().trim());
	}

	private static String formatSupply(final BigDecimal value) {
		return String.format(Locale.US, "%,.6f", value);
	}

	public static void main(final String[] args) {
		try {
			new QueryCurrencySupply().run();
		} catch (final Exception ex) {
			System.out.println(ex.getMessage());
		}
	}

	private void run() throws IOException, InterruptedException {
		System.out.printf("Using node %s%n", NODE_URL);

		// [>step-1]
		final BigDecimal maximumSupply = fetchSupplyValue("max");
		System.out.printf("Maximum supply: %s XYM%n",
			formatSupply(maximumSupply));

		final BigDecimal totalSupply = fetchSupplyValue("total");
		System.out.printf("Total supply: %s XYM%n",
			formatSupply(totalSupply));

		final BigDecimal circulatingSupply =
			fetchSupplyValue("circulating");
		System.out.printf("Circulating supply: %s XYM%n",
			formatSupply(circulatingSupply)); // [<step-1]
		// [>step-2]
		final BigDecimal nonCirculatingSupply =
			totalSupply.subtract(circulatingSupply);
		System.out.printf("Non-circulating supply: %s XYM%n",
			formatSupply(nonCirculatingSupply));

		final BigDecimal unmintedSupply =
			maximumSupply.subtract(totalSupply);
		System.out.printf("Unminted supply: %s XYM%n",
			formatSupply(unmintedSupply)); // [<step-2]
	}
}
