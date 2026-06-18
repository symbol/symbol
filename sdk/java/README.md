# Symbol Java SDK

Java SDK for the Symbol and NEM blockchains: transaction building, signing, verification,
key derivation, and message encryption. Wire-format and behavioral compatibility with the
other Symbol SDKs is pinned by the shared cross-SDK test vectors (`tests/vectors`).

## Requirements

- Java 21 or newer
- Gradle (a wrapper is not committed; the project is built with the system Gradle)

## Build

```sh
gradle build
```

## Test

```sh
gradle test
```

## Lint / format

The project uses [Spotless](https://github.com/diffplug/spotless) configured to
preserve the project's tabs-and-140-cols style. Run:

```sh
gradle spotlessCheck   # verify
gradle spotlessApply   # auto-fix
```

## Cross-language vectors

```sh
BLOCKCHAIN=symbol gradle vectors
BLOCKCHAIN=nem    gradle vectors
gradle catVectors
```

## Code generation

The catbuffer model classes under `org.symbol.sdk.nem.models` and
`org.symbol.sdk.symbol.models` are generated from the catbuffer schemas by the Python plugin
in [`generator/`](generator), invoked through the [`catparser`](../../catbuffer/parser) tool.
The generator emits one `.java` file per type (POD / enum / struct / factory) into a dedicated
`models/` subpackage, keeping the hand-written runtime classes (`Address`, `KeyPair`,
`SymbolFacade`, ...) in the parent `org.symbol.sdk.{nem,symbol}` package uncluttered. A few
hand-written runtime types share a name with a catbuffer type (currently only `Address`): these
are the high-level facade forms and are distinct from the generated wire-format model of the same
name (e.g. the 24-byte `models.Address` POD vs. the base32 `symbol.Address` facade), so both are
emitted — the model into `models/`, the facade in the parent package — mirroring the other SDKs.

To regenerate after a schema change, either run the script directly:

```sh
./scripts/run_catbuffer_generator.sh
```

or invoke the Gradle wrapper task (which shells out to the same script):

```sh
gradle generateModels
```

The Python templates emit canonical but unwrapped Java; line-wrapping and whitespace are owned
by Spotless (the eclipse formatter configured in `eclipse-formatter.xml`). Both regeneration paths
therefore run `spotlessApply` over the freshly generated tree, so committed generated files stay
lint-clean without the templates having to replicate the formatter's wrapping rules.

A `dryrun` mode is also available — it emits into a throwaway `<blockchain>_dryrun/`
package and deletes it afterwards, useful for verifying the generator runs cleanly
without touching tracked files:

```sh
./scripts/run_catbuffer_generator.sh dryrun
```

## Crypto implementation

Cryptographic primitives use native JDK APIs whenever possible:

| Primitive          | Implementation                                                                  |
| ------------------ | ------------------------------------------------------------------------------- |
| SHA-256, SHA-512   | `java.security.MessageDigest`                                                   |
| HMAC-SHA512        | `javax.crypto.Mac`                                                              |
| AES-CBC, AES-GCM   | `javax.crypto.Cipher`                                                           |
| Ed25519 (SHA-512)  | `java.security.Signature` ("Ed25519")                                           |
| Keccak-256/-512    | Bouncy Castle (`org.bouncycastle.crypto.digests.KeccakDigest`)                  |
| RIPEMD-160         | Bouncy Castle (`org.bouncycastle.crypto.digests.RIPEMD160Digest`)               |
| HKDF-SHA256        | Bouncy Castle (`org.bouncycastle.crypto.generators.HKDFBytesGenerator`)         |
| Ed25519 (Keccak)   | Bouncy Castle low-level `Ed25519` with hasher swap; small ported helpers        |
| BIP-32 / BIP-39    | Hand-rolled (PBKDF2-SHA512 via `javax.crypto.SecretKeyFactory`) + bundled lists |

## SDK runtime

In addition to the generated catbuffer model classes, `sdk/java` provides a hand-written
runtime layer. The high-level entry points
are [`SymbolFacade`](src/main/java/org/symbol/sdk/symbol/SymbolFacade.java) and
[`NemFacade`](src/main/java/org/symbol/sdk/nem/NemFacade.java); they compose the
following pieces:

| Layer            | Class(es)                                                                                              |
| ---------------- | ------------------------------------------------------------------------------------------------------ |
| Networks         | `org.symbol.sdk.{symbol,nem}.Network`, `NetworkTimestamp`, `Address`                                   |
| Cryptography     | `org.symbol.sdk.{symbol,nem}.{KeyPair,Verifier,SharedKey}`, `org.symbol.sdk.Bip32`                     |
| Messages         | `org.symbol.sdk.{symbol,nem}.MessageEncoder`, `org.symbol.sdk.MessageEncoderResult`                    |
| Descriptors      | `org.symbol.sdk.{TransactionDescriptorProcessor,RuleBasedTransactionFactory}`                          |
| Transactions     | `org.symbol.sdk.{symbol,nem}.{SymbolTransactionFactory,NemTransactionFactory}`                         |
| Symbol-only      | `IdGenerator`, `Merkle`, `Metadata`, `Restriction`, `VotingKeysGenerator`                              |

The SDK exposes two descriptor surfaces:

- **Typed descriptors** under `org.symbol.sdk.{symbol,nem}.descriptors` — strongly typed,
  IDE-completable, and the recommended entry point. Each `XxxDescriptor` is a thin wrapper
  over a `Map<String, Object>` (accessible via `toMap()`) so it composes naturally with the
  dynamic path.
- **Dynamic `Map<String, Object>` descriptors** for cases where the transaction shape is
  data-driven (forms, JSON, etc.). Same wire format; same fee/deadline plumbing.

## Usage

```java
import org.symbol.sdk.CryptoTypes;
import org.symbol.sdk.symbol.SymbolFacade;
import org.symbol.sdk.symbol.descriptors.TransferTransactionV1Descriptor;
import org.symbol.sdk.symbol.descriptors.UnresolvedMosaicDescriptor;
import org.symbol.sdk.symbol.models.Amount;
import org.symbol.sdk.symbol.models.Signature;
import org.symbol.sdk.symbol.models.Transaction;
import org.symbol.sdk.symbol.models.UnresolvedMosaicId;

SymbolFacade facade = new SymbolFacade("testnet");
SymbolFacade.SymbolAccount account = facade.createAccount(
        new CryptoTypes.PrivateKey("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));

// Each descriptor takes its required fields as constructor arguments and exposes a fluent
// setter for every optional field. Both the constructor and the setters come in a typed and a
// string-parsing flavour (nested descriptors included), and array setters accept a List or varargs.

// 1. typed form — pass the SDK types directly
TransferTransactionV1Descriptor descriptor = new TransferTransactionV1Descriptor(
        new Address("TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I"))
        .mosaics(new UnresolvedMosaicDescriptor(
                new UnresolvedMosaicId(java.math.BigInteger.valueOf(0x7CDF3B117A3C40CCL)),
                new Amount(java.math.BigInteger.valueOf(1_000_000L))))
        .message("hello symbol".getBytes(java.nio.charset.StandardCharsets.UTF_8));

// 2. string form — every field accepts its canonical string form, nested descriptors included.
//    Integer ids accept 0x-prefixed hex (or decimal); amounts are usually given in decimal.
TransferTransactionV1Descriptor stringDescriptor = new TransferTransactionV1Descriptor(
        "TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I")
        .mosaics(new UnresolvedMosaicDescriptor("0x7CDF3B117A3C40CC", "1000000"))
        .message("hello symbol");

// 3. array input — pass multiple elements as varargs (or a List) to the array setter
TransferTransactionV1Descriptor multiMosaicDescriptor = new TransferTransactionV1Descriptor(
        "TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I")
        .mosaics(
                new UnresolvedMosaicDescriptor("0x7CDF3B117A3C40CC", "1000000"),
                new UnresolvedMosaicDescriptor("0x1F031D8D3905B931", "5"))
        .message("hello symbol");

Transaction transaction = facade.createTransactionFromTypedDescriptor(
        descriptor, account.publicKey, /* feeMultiplier */ 100L, /* deadlineSeconds */ 60L);
CryptoTypes.Signature signature = account.signTransaction(transaction);
transaction.setSignature(new Signature(signature.bytes));

CryptoTypes.Hash256 transactionHash = facade.hashTransaction(transaction);
assert facade.verifyTransaction(transaction, signature);
```

### Data-driven (JSON)

The same transfer from a JSON document — `u64` ids/amounts may be JSON numbers, decimal
strings, or `0x`-hex strings (read losslessly as `BigInteger`):

```java
String json = """
        {
            "type": "transfer_transaction_v1",
            "recipientAddress": "TCHBDENCLKEBILBPWP3JPB2XNY64OE7PYHHE32I",
            "mosaics": [{"mosaicId": "0x7CDF3B117A3C40CC", "amount": 1000000}],
            "message": "hello symbol"
        }""";
Transaction jsonTransaction = facade.createTransactionFromJson(
        json, account.publicKey, /* feeMultiplier */ 100L, /* deadlineSeconds */ 60L);
```

A `Map<String, Object>` works the same way via `facade.createTransactionFromDescriptor(map, …)` —
identical wire format. See [`SymbolReadme.java`](examples/src/main/java/org/symbol/examples/readme/SymbolReadme.java)
for the typed / JSON / map paths side by side.

### NEM

NEM works the same way via `NemFacade`, except the fee is an absolute `BigInteger` rather than a
fee multiplier. Descriptors and their setters share the same typed/string flavours:

```java
import org.symbol.sdk.nem.NemFacade;
import org.symbol.sdk.nem.descriptors.MessageDescriptor;
import org.symbol.sdk.nem.descriptors.TransferTransactionV1Descriptor; // note: nem.descriptors

NemFacade nemFacade = new NemFacade("testnet");
NemFacade.NemAccount nemAccount = nemFacade.createAccount(
        new CryptoTypes.PrivateKey("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"));

// string form: base32 address, decimal amount, nested message descriptor
TransferTransactionV1Descriptor nemTransfer = new TransferTransactionV1Descriptor(
        "TALICEROONSJCPHC63F52V6FY3SDMSVAEUGHMB7C", "5000000")
        .message(new MessageDescriptor("plain").message("hello nem"));

Transaction nemTransaction = nemFacade.createTransactionFromTypedDescriptor(
        nemTransfer, nemAccount.publicKey,
        /* fee */ java.math.BigInteger.valueOf(100_000L), /* deadlineSeconds */ 60L);
```
