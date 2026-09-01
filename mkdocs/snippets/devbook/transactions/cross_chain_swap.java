//JAVA 21+
//DEPS org.web3j:core:4.12.3
//DEPS org.symbol:symbol-sdk:3.3.1

import java.io.IOException;
import java.math.BigInteger;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.net.http.HttpResponse.BodyHandlers;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Arrays;
import java.util.HexFormat;
import java.util.List;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.web3j.abi.FunctionEncoder;
import org.web3j.abi.FunctionReturnDecoder;
import org.web3j.abi.TypeReference;
import org.web3j.abi.datatypes.Bool;
import org.web3j.abi.datatypes.DynamicBytes;
import org.web3j.abi.datatypes.Function;
import org.web3j.abi.datatypes.generated.Bytes32;
import org.web3j.abi.datatypes.generated.Uint256;
import org.web3j.crypto.Credentials;
import org.web3j.protocol.Web3j;
import org.web3j.protocol.core.DefaultBlockParameterName;
import org.web3j.protocol.core.methods.response.EthCall;
import org.web3j.protocol.core.methods.response.TransactionReceipt;
import org.web3j.protocol.http.HttpService;
import org.web3j.tx.RawTransactionManager;
import org.web3j.tx.response.PollingTransactionReceiptProcessor;
import org.web3j.utils.Convert;
import org.web3j.utils.Numeric;

import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.facade.SymbolFacade;
import org.symbol.sdk.symbol.Address;
import org.symbol.sdk.symbol.IdGenerator;
import org.symbol.sdk.symbol.KeyPair;
import org.symbol.sdk.symbol.SymbolTransactionFactory;
import org.symbol.sdk.symbol.descriptors.*;
import org.symbol.sdk.symbol.models.*;
import org.symbol.sdk.utils.Converter;

final class CrossChainSwap {
	private static final ObjectMapper JSON_MAPPER = new ObjectMapper();

	private static final HttpClient HTTP_CLIENT =
		HttpClient.newHttpClient();

	private static final SecureRandom RANDOM = new SecureRandom();

	private final String symbolNodeUrl = System.getenv().getOrDefault(
		"SYMBOL_NODE_URL", "https://reference.symboltest.net:3001");

	private final String ethRpcUrl = System.getenv().getOrDefault(
		"ETH_RPC_URL", "https://ethereum-sepolia-rpc.publicnode.com");

	// Ethereum HTLC contract on Sepolia
	private static final String HTLC_ADDRESS =
		"0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B";

	private final SymbolFacade facade = new SymbolFacade("testnet");

	private Web3j ethProvider;

	private Credentials aliceEthWallet;

	private Credentials bobEthWallet;

	private Address aliceXymAddress;

	private Address bobXymAddress;

	private KeyPair aliceXymKeyPair;

	private KeyPair bobXymKeyPair;

	public static void main(final String[] args) {
		try {
			new CrossChainSwap().run();
		} catch (final Exception ex) {
			System.out.println(null == ex.getMessage()
				? ex.toString()
				: ex.getMessage());
		}
	}

	private void run()
		throws Exception {
		System.out.printf("Using Symbol node %s%n", symbolNodeUrl);
		System.out.printf("Using Ethereum RPC %s%n", ethRpcUrl);

		// Symbol accounts [>step-1]
		// Alice (creates the ETH lock, claims XYM on Symbol)
		final String aliceXymPrivateKey = System.getenv().getOrDefault(
			"ALICE_XYM_PRIVATE_KEY", "0".repeat(64));
		aliceXymKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(aliceXymPrivateKey));
		aliceXymAddress = facade.network.publicKeyToAddress(
			aliceXymKeyPair.getPublicKey());
		System.out.printf("Alice Symbol address: %s%n", aliceXymAddress);

		// Bob (creates the XYM lock, claims ETH on Ethereum)
		final String bobXymPrivateKey = System.getenv().getOrDefault(
			"BOB_XYM_PRIVATE_KEY", "1".repeat(64));
		bobXymKeyPair = new KeyPair(
			new CryptoTypes.PrivateKey(bobXymPrivateKey));
		bobXymAddress = facade.network.publicKeyToAddress(
			bobXymKeyPair.getPublicKey());
		System.out.printf("Bob Symbol address: %s%n", bobXymAddress);

		// Ethereum accounts
		ethProvider = Web3j.build(new HttpService(ethRpcUrl));

		final String aliceEthPrivateKey = System.getenv().getOrDefault(
			"ALICE_ETH_PRIVATE_KEY",
			"a73276699ba72dc7b5c9d08deaf8cd88eec33c866341b12030443"
				+ "2b89586d45d");
		aliceEthWallet = Credentials.create(aliceEthPrivateKey);
		System.out.printf("Alice ETH address: %s%n",
			aliceEthWallet.getAddress());

		final String bobEthPrivateKey = System.getenv().getOrDefault(
			"BOB_ETH_PRIVATE_KEY",
			"8e85561005f27d926af79a7ce3e76e75108a09ff2ab78bf65b557"
				+ "8d2e5d509bf");
		bobEthWallet = Credentials.create(bobEthPrivateKey);
		System.out.printf("Bob ETH address: %s%n",
			bobEthWallet.getAddress());
		// [<step-1]

		// --- Alice: Generate proof and hashlock ---
		System.out.println( // [>step-2]
			"\n--- Alice: Generate proof and hashlock ---");

		final byte[] proof = new byte[32];
		RANDOM.nextBytes(proof);
		System.out.printf("Proof (hex): %s%n",
			HexFormat.of().formatHex(proof));

		final byte[] firstHash = sha256(proof);
		final byte[] secret = sha256(firstHash);
		System.out.printf("Secret (double SHA-256): %s%n",
			HexFormat.of().formatHex(secret));
		// [<step-2]
		// --- Step 1. Alice: Lock ETH on Ethereum ---
		System.out.println( // [>step-3]
			"\n--- Step 1. Alice: Lock ETH on Ethereum ---");

		final long timelock =
			System.currentTimeMillis() / 1000 + (72 * 60 * 60);
		System.out.printf("Ethereum timelock (Unix): %d%n", timelock);

		final String lockHash = sendNewContract(
			bobEthWallet.getAddress(),
			secret,
			BigInteger.valueOf(timelock));
		System.out.printf("Lock TX hash: %s%n", lockHash);

		final TransactionReceipt lockReceipt = waitForEthereumReceipt(
			lockHash);
		System.out.printf("Lock confirmed in block %s%n",
			lockReceipt.getBlockNumber());

		// Extract the contractId from the LogHTLCNew event
		final String contractId = lockReceipt.getLogs().get(0)
			.getTopics().get(1);
		System.out.printf("HTLC contract ID: %s%n", contractId);
		// [<step-3]
		// --- Step 2. Bob: Create secret lock on Symbol ---
		System.out.println( // [>step-4]
			"\n--- Step 2. Bob: Create secret lock on Symbol ---");

		// Bob queries the Ethereum contract to get the hashlock
		final String hashlock = getContractHashlock(contractId);
		System.out.printf("Hashlock from chain: %s%n", hashlock);

		final long lockDuration = 5760; // ~48h at 30s blocks
		System.out.printf("Lock duration: %d blocks%n", lockDuration);

		final Transaction secretLockTransaction =
			facade.createTransactionFromTypedDescriptor(
				new SecretLockTransactionV1Descriptor(
					aliceXymAddress,
					new CryptoTypes.Hash256(hashlock),
					new UnresolvedMosaicDescriptor(
						new UnresolvedMosaicId(
							IdGenerator.generateMosaicAliasId(
								"symbol.xym")),
						new Amount(1_000_000)), // 1 XYM
					new BlockDuration(lockDuration),
					LockHashAlgorithm.HASH_256),
				bobXymKeyPair.getPublicKey(),
				getFeeMultiplier(),
				2 * 60 * 60);

		// Sign and announce
		final CryptoTypes.Signature lockSignature =
			facade.signTransaction(bobXymKeyPair, secretLockTransaction);
		final String lockPayload = SymbolTransactionFactory
			.attachSignature(secretLockTransaction, lockSignature);

		System.out.println("Built secret lock transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(secretLockTransaction.toJson()));

		final String symbolLockHash = facade.hashTransaction(
			secretLockTransaction).toString();
		System.out.printf("Secret lock transaction hash: %s%n",
			symbolLockHash);
		announceTransaction(lockPayload, "/transactions", "secret lock");
		waitForStatus(symbolLockHash, "confirmed", "Secret lock");
		// [<step-4]
		// --- Step 3. Alice: Claim XYM on Symbol ---
		System.out.println( // [>step-5]
			"\n--- Step 3. Alice: Claim XYM on Symbol ---");

		final Transaction secretProofTransaction =
			facade.createTransactionFromTypedDescriptor(
				new SecretProofTransactionV1Descriptor(
					aliceXymAddress,
					new CryptoTypes.Hash256(hashlock),
					LockHashAlgorithm.HASH_256,
					proof),
				aliceXymKeyPair.getPublicKey(),
				getFeeMultiplier(),
				2 * 60 * 60);

		// Sign and announce
		final CryptoTypes.Signature proofSignature =
			facade.signTransaction(
				aliceXymKeyPair, secretProofTransaction);
		final String proofPayload = SymbolTransactionFactory
			.attachSignature(secretProofTransaction, proofSignature);

		System.out.println("Built secret proof transaction:");
		System.out.println(JSON_MAPPER.writerWithDefaultPrettyPrinter()
			.writeValueAsString(secretProofTransaction.toJson()));

		final String proofHash = facade.hashTransaction(
			secretProofTransaction).toString();
		System.out.printf("Secret proof transaction hash: %s%n",
			proofHash);
		announceTransaction(proofPayload, "/transactions", "secret proof");
		waitForStatus(proofHash, "confirmed", "Secret proof");
		// [<step-5]
		// --- Step 4. Bob: Withdraw ETH on Ethereum ---
		System.out.println( // [>step-6]
			"\n--- Step 4. Bob: Withdraw ETH on Ethereum ---");

		// Bob waits for Alice to reveal the proof on Symbol.
		final byte[] revealedProof = waitForSecretProof(
			aliceXymAddress.toString(), hashlock);
		System.out.printf("Proof from chain: %s%n",
			HexFormat.of().formatHex(revealedProof));

		final String withdrawHash = sendWithdraw(
			contractId, revealedProof);
		System.out.printf("Withdraw TX hash: %s%n", withdrawHash);

		final TransactionReceipt withdrawReceipt = waitForEthereumReceipt(
			withdrawHash);
		System.out.printf("Withdraw confirmed in block %s%n",
			withdrawReceipt.getBlockNumber());
		// [<step-6]
		System.out.println("\n--- Cross-chain swap complete ---");
	}

	// Helper function to fetch recommended Symbol fee multiplier
	private long getFeeMultiplier()
		throws IOException, InterruptedException {
		final String feePath = "/network/fees/transaction";
		System.out.printf("Fetching recommended fees from %s%n", feePath);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(symbolNodeUrl + feePath)).GET().build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		final JsonNode feeJSON = JSON_MAPPER.readTree(response.body());
		final long medianMultiplier =
			feeJSON.get("medianFeeMultiplier").asLong();
		final long minimumMultiplier =
			feeJSON.get("minFeeMultiplier").asLong();
		final long feeMultiplier = Math.max(
			medianMultiplier, minimumMultiplier);
		System.out.printf("  Fee multiplier: %d%n", feeMultiplier);
		return feeMultiplier;
	}

	// Helper function to announce a Symbol transaction
	private void announceTransaction(
		final String payload,
		final String endpoint,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf("Announcing %s to %s%n", label, endpoint);
		final HttpRequest request = HttpRequest.newBuilder(
			URI.create(symbolNodeUrl + endpoint))
			.header("Content-Type", "application/json")
			.PUT(HttpRequest.BodyPublishers.ofString(payload))
			.build();
		final HttpResponse<String> response = HTTP_CLIENT.send(
			request, BodyHandlers.ofString());
		System.out.printf("  Response: %s%n", response.body());
	}

	// Helper function to wait for Symbol transaction status
	private void waitForStatus(
		final String hash,
		final String expectedStatus,
		final String label
	) throws IOException, InterruptedException {
		System.out.printf(
			"Waiting for %s to reach %s status...%n",
			label, expectedStatus);
		int attempts = 0;
		final int maxAttempts = 60;

		while (attempts < maxAttempts) {
			final String url = symbolNodeUrl + "/transactionStatus/"
				+ hash;
			final HttpRequest request = HttpRequest.newBuilder(
				URI.create(url)).GET().build();
			final HttpResponse<String> response = HTTP_CLIENT.send(
				request, BodyHandlers.ofString());

			if (response.statusCode() / 100 != 2) {
				if (404 == response.statusCode()) {
					System.out.println(
						"  Transaction status: not yet available");
				} else
					throw new IOException(
						"HTTP " + response.statusCode());
			} else {
				final JsonNode status = JSON_MAPPER.readTree(
					response.body());
				System.out.printf("  Transaction status: %s%n",
					status.get("group").asText());

				if ("failed".equals(status.get("group").asText()))
					throw new IOException(String.format("%s failed: %s",
						label, status.get("code").asText()));

				if (status.get("group").asText().equals(expectedStatus)) {
					System.out.printf("%s %s in %d seconds%n",
						label, expectedStatus, attempts);
					return;
				}
			}

			++attempts;
			Thread.sleep(1000);
		}

		throw new IOException(String.format(
			"%s not %s after %d attempts",
			label, expectedStatus, maxAttempts));
	}

	// Poll Symbol for a confirmed secret proof transaction matching
	// a hashlock.
	private byte[] waitForSecretProof(
		final String signerAddress,
		final String hashlock
	) throws IOException, InterruptedException {
		final String hashlockHex = hashlock.toUpperCase();
		final String url = symbolNodeUrl + "/transactions/confirmed"
			+ "?address=" + signerAddress + "&type=16978&order=desc";
		System.out.printf("Polling %s%n", url);
		System.out.printf("  Looking for secret: %s%n", hashlockHex);

		int attempts = 0;
		final int maxAttempts = 60;
		while (attempts < maxAttempts) {
			final HttpRequest request = HttpRequest.newBuilder(
				URI.create(url)).GET().build();
			final HttpResponse<String> response = HTTP_CLIENT.send(
				request, BodyHandlers.ofString());
			final JsonNode data = JSON_MAPPER.readTree(response.body());
			for (final JsonNode tx : data.get("data")) {
				final String secret = tx.get("transaction")
					.get("secret").asText("").toUpperCase();
				if (secret.equals(hashlockHex)) {
					System.out.printf(
						"  Found proof transaction after %ds%n",
						attempts);
					return Converter.hexToUint8(tx.get("transaction")
						.get("proof").asText());
				}
			}
			++attempts;
			Thread.sleep(1000);
		}

		throw new IOException(String.format(
			"Secret proof not found after %d attempts", maxAttempts));
	}

	private String sendNewContract(
		final String receiver,
		final byte[] hashlock,
		final BigInteger timelock
	) throws Exception {
		final Function function = new Function(
			"newContract",
			List.of(
				new org.web3j.abi.datatypes.Address(receiver),
				new Bytes32(hashlock),
				new Uint256(timelock)),
			List.of(new TypeReference<Bytes32>() {})
		);
		final String data = FunctionEncoder.encode(function);
		final RawTransactionManager manager = new RawTransactionManager(
			ethProvider, aliceEthWallet);
		return manager.sendTransaction(
			BigInteger.valueOf(20_000_000_000L),
			BigInteger.valueOf(300_000),
			HTLC_ADDRESS,
			data,
			Convert.toWei("0.01", Convert.Unit.ETHER).toBigInteger()
		).getTransactionHash();
	}

	private String getContractHashlock(
		final String contractId
	) throws IOException {
		final Function function = new Function(
			"getContract",
			List.of(new Bytes32(Numeric.hexStringToByteArray(contractId))),
			Arrays.asList(
				new TypeReference<org.web3j.abi.datatypes.Address>() {},
				new TypeReference<org.web3j.abi.datatypes.Address>() {},
				new TypeReference<Uint256>() {},
				new TypeReference<Bytes32>() {},
				new TypeReference<Uint256>() {},
				new TypeReference<Bool>() {},
				new TypeReference<Bool>() {},
				new TypeReference<DynamicBytes>() {})
		);
		final String data = FunctionEncoder.encode(function);
		final EthCall response = ethProvider.ethCall(
			org.web3j.protocol.core.methods.request.Transaction
				.createEthCallTransaction(
					bobEthWallet.getAddress(), HTLC_ADDRESS, data),
			DefaultBlockParameterName.LATEST
		).send();
		return Numeric.toHexStringNoPrefix((byte[]) FunctionReturnDecoder
			.decode(response.getValue(), function.getOutputParameters())
			.get(3).getValue());
	}

	private String sendWithdraw(
		final String contractId,
		final byte[] revealedProof
	) throws Exception {
		final Function function = new Function(
			"withdraw",
			List.of(
				new Bytes32(Numeric.hexStringToByteArray(contractId)),
				new DynamicBytes(revealedProof)),
			List.of(new TypeReference<Bool>() {})
		);
		final String data = FunctionEncoder.encode(function);
		final RawTransactionManager manager = new RawTransactionManager(
			ethProvider, bobEthWallet);
		return manager.sendTransaction(
			BigInteger.valueOf(20_000_000_000L),
			BigInteger.valueOf(200_000),
			HTLC_ADDRESS,
			data,
			BigInteger.ZERO
		).getTransactionHash();
	}

	private TransactionReceipt waitForEthereumReceipt(
		final String transactionHash
	) throws Exception {
		return new PollingTransactionReceiptProcessor(
			ethProvider, 1000, 60).waitForTransactionReceipt(
				transactionHash);
	}

	private static byte[] sha256(
		final byte[] data
		) throws NoSuchAlgorithmException {
			return MessageDigest.getInstance("SHA-256").digest(data);
		}
	}
