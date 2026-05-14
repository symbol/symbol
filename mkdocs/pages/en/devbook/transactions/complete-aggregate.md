---
title: Complete Aggregate
tutorial_level: advanced
---

# Creating a Complete Aggregate Transaction

This tutorial shows how to create an asset swap using <complete aggregate transactions:>.

In this example, Account A sends 10 <XYM:> to Account B, while Account B simultaneously sends 1 custom <mosaic:> back
to Account A:

```dot
digraph {
    rankdir="LR";
    fontsize=12;
    subgraph clusterAggregate {
        label = "Complete Aggregate Transaction";
        tooltip = "Complete Aggregate Transaction";
        subgraph clusterT1 {
            label = "Embedded Transfer 1";
            tooltip = "Embedded Transfer 1";
            style = dashed;
            A2 [label="Account A" tooltip="Account A"];
            B2 [label="Account B" tooltip="Account B"];
            A2 -> B2 [label="10 XYM"];
        }
        subgraph clusterT2 {
            label = "Embedded Transfer 2";
            tooltip = "Embedded Transfer 2";
            style = dashed;
            A1 [label="Account A" tooltip="Account A"];
            B1 [label="Account B" tooltip="Account B"];
            A1 -> B1 [label="1 Custom Mosaic" dir=back];
        }
    }
}
```

Both parties coordinate off-chain to collect signatures before announcement, ensuring the swap executes as a
single atomic transaction.

!!! note "Two types of aggregate transactions"

    <Aggregate transactions:|Aggregate transactions> group multiple <transactions:> in a single operation,
    and require <signatures:> from all involved accounts.

    A _complete aggregate transaction_ collects all signatures before being announced.
    This procedure works well, for example, in the following scenarios:

    * **Multi-party coordination:** When parties can communicate off-chain to exchange transaction payloads and
      <cosignatures:>.
    * **Single-account batching:** When one account wants to execute multiple transactions atomically.
      No cosignatures are needed in this case.

    If off-chain coordination is impractical, use <bonded aggregate transactions:> instead,
    which allow cosignatures to be collected on the network.

## Prerequisites

Before you start, make sure to set up your development environment.
See [Setting Up a Development Environment](../start/setup.md).

You also need two <accounts:> with <XYM:> and one custom <mosaic:> to complete the swap.
Although pre-funded accounts are provided for convenience, they are not maintained and may run out of funds.

To use your own accounts, complete the following steps:

* Create an account (Account A) to initiate the aggregate transaction, either
  [from code](../accounts/create-from-private-key.md) or
  [by using a wallet](../../userbook/wallet/create-account.md).
* Create a second account (Account B) to participate in the swap.
* Obtain XYM for Account A to pay for the transaction fee and transfer amounts.
  See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).
* Create a mosaic owned by Account B for the swap.
  See [Creating a Mosaic](../mosaics/create-mosaic.md).

Additionally, review the [Transfer transaction](./transfer.md) tutorial to understand how
transactions are announced and confirmed.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/transactions/complete_aggregate', ['py', 'js']) }}

The whole code is wrapped in a single `try` block to provide simple error handling,
but applications will probably want to use more fine-grained control.

A complete aggregate transaction may involve two distinct roles: an **initiator** (Account A) that builds and announces
the aggregate, and one or more **cosigners** (Account B, and any additional cosigners) that receive the transaction
payload off-chain and add their signatures after verifying the transaction.
When only one account is involved, no cosignatures are needed.

In practice, each role runs as a separate program on a separate machine.
This tutorial demonstrates the multi-party case but combines both roles in a single script for simplicity.

## Account A: Initiator Workflow

### Setting Up Accounts

{{ tutorial.code_snippet_tagged('step-1') }}

This example includes both <private keys:> in one script for simplicity.
In practice, each party signs on their own machine.
Account A only needs Account B's public key to build the aggregate, because B's <public key:> is required to set B as
the signer of an embedded transaction and to derive B's <address:>.

The `ACCOUNT_A_PRIVATE_KEY` and `ACCOUNT_B_PRIVATE_KEY` environment variables set the keys for each account.
If not provided, test keys are used as defaults.
If using your own keys, ensure Account A has XYM and Account B holds a custom mosaic for the swap.

The addresses for both accounts are derived from their public keys using the facade's network configuration.

### Fetching Network Time and Fees

{{ tutorial.code_snippet_tagged('step-2') }}

Network time and recommended fees are fetched from <get:/node/time> and <get:/network/fees/transaction> respectively,
following the process described in the [Transfer Transaction](./transfer.md) tutorial.

### Creating Embedded Transactions

{{ tutorial.code_snippet_tagged('step-3') }}

The <embedded transactions:> define the operations to execute atomically.
Each embedded transaction specifies:

* **Type:** All transaction types can be embedded within aggregates (except other aggregates).
  For embedded transfers, use `transfer_transaction_v1`, the same as for basic transfer transactions.

* **Signer public key:** The account that would sign this transaction if it were announced
  independently.

* **Transaction-specific fields:** All fields specific to the transaction type must be provided.
  For transfers, this includes the recipient address and the mosaics to send.

Note that embedded transactions do **not** include fee or deadline fields.
These are inherited from the enclosing aggregate transaction.

The example creates two <transfer transactions:> for the swap:

* The first transfer sends 10 XYM from Account A to Account B.
* The second transfer sends 1 custom mosaic from Account B to Account A.

!!! note "About the custom mosaic"

    The custom mosaic with ID `0x6D1314BE751B62C2` was created for this tutorial.
    The default Account B has been seeded with this mosaic so the swap can execute successfully.

    If using your own accounts, ensure Account B holds a custom mosaic and update the mosaic ID in the code.

### Building the Aggregate Transaction

{{ tutorial.code_snippet_tagged('step-4') }}

Once the embedded transactions are prepared, create the complete aggregate transaction that wraps them:

* **Type:** Use <ser:AggregateCompleteTransactionV3|aggregate_complete_transaction_v3>.

* **Signer public key:** The account initiating the aggregate.
  This account announces the transaction and pays the transaction fee.

    !!! tip "Sharing transaction fees"

        While the signer pays the entire fee upfront, other participants can contribute to the cost within
        the aggregate.
        For more details, see the [Paying Transaction Fees on Behalf of Another Account](./fee-sponsorship.md) tutorial.

* **Deadline:** The timestamp, in [network time](./transfer.md#fetching-network-time), after which the transaction
  expires and can no longer be confirmed.

* **Transactions hash:** A hash computed from all embedded transactions.
  This ensures the embedded transactions cannot be modified after signing.
  Use <dy:SymbolFacade.hashEmbeddedTransactions> to compute this value.

* **Transactions:** The array of embedded transactions to execute.

The fee is calculated based on the aggregate's total size, which includes all embedded transactions plus
space reserved for one cosignature.

### Signing the Transaction

{{ tutorial.code_snippet_tagged('step-5') }}

Account A signs the transaction using <dy:SymbolFacade.signTransaction> and produces an intermediate payload using
<dy:SymbolTransactionFactory.attachSignature>.
This payload is not yet ready to announce because it is missing Account B's cosignature.
Account A sends this intermediate payload to Account B through an off-chain channel.

!!! note "Signatures in aggregate transactions"

    An account only signs once, even if it appears as the signer in multiple embedded transactions.
    In this tutorial, Account A signs the aggregate transaction, which covers both the aggregate itself and the
    first embedded transaction where Account A is the signer.

    When all embedded transactions share the same signer (batching multiple operations from one account),
    cosignatures are **not** required. The aggregate can be announced immediately after signing, and the fee
    calculation does not need to reserve space for cosignatures.

## Account B: Cosigner Workflow

### Verifying and Cosigning

{{ tutorial.code_snippet_tagged('step-6') }}

Account B receives the payload and deserializes it using <dy:SymbolTransactionFactory.deserialize> to reconstruct the
transaction object.
Account B should verify that the embedded transactions match what it expects to sign.

It then cosigns using <dy:SymbolFacade.cosignTransaction>, which computes the transaction hash and produces a
cosignature object.
Only this cosignature is sent back to Account A.

!!! warning "Verify before cosigning"

    Always inspect transaction content before cosigning.
    Cosignatures are binding and cannot be undone.

## Account A: Announcing and Confirming

### Collecting the Cosignature

{{ tutorial.code_snippet_tagged('step-7') }}

Account A receives Account B's cosignature, adds it to the transaction object's `cosignatures` array, and rebuilds the
payload for announcement.

### Announcing the Transaction

Now that the transaction is ready to be announced, it follows the same process as regular, non-aggregate transactions,
as shown in the [Transfer Transaction](./transfer.md#announcing-the-transaction) tutorial.

{{ tutorial.code_snippet_tagged('step-8') }}

Once all signatures are collected, the transaction is announced to a <node:> using the <put:/transactions> endpoint.

The node validates that all required signatures are present and valid before accepting the transaction.
If validation passes, the transaction is added to the <unconfirmed pool:> and broadcast to other nodes.

### Waiting for Confirmation

{{ tutorial.code_snippet_tagged('step-9') }}

After announcement, the transaction status is monitored using <get:/transactionStatus/{hash}>.

The polling loop checks the status every second until the transaction is confirmed or fails.
Once confirmed, the swap is complete and both transfers have executed.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="10 14 18 48 53 60 66"
--8<-- 'devbook/transactions/complete_aggregate.log'
```

Key points in the output:

* **Line 10** (`"signature": "0000..."`): Shows all zeros initially because the transaction hasn't been signed yet.
* **Line 14** (`"type": 16705`): Indicates this is an <ser:AggregateCompleteTransactionV3>.
* **Line 18** (`"transactions"`): Contains the two embedded transfers that will execute atomically.
* **Line 48** (`"cosignatures": []`): Initially empty. Account B's cosignature is added before announcement.
    Note how Account A's signature is only needed once, even though it appears as signer in both the aggregate and the
    first embedded transaction.
* **Line 53** (`"payload": "6801..."`): The transaction payload computed from the aggregate transaction and its embedded
    transactions.
* **Line 60** (`"signature": "7037..."`): Account B's cosignature for the aggregate transaction.
* **Line 66** (`Waiting for confirmation ...`): The hash shown in the confirmation check can be used to search for the
    transaction in the [Symbol Testnet Explorer](https://testnet.symbol.fyi/).

The aggregate transaction is treated as a single atomic unit by the network.
The swap executes completely: Account A receives the custom mosaic and Account B receives the XYM,
or the entire transaction fails and no assets are transferred.

## Conclusion

This tutorial showed how to:

| Step                                                            | Related documentation                                                               |
|-----------------------------------------------------------------|-------------------------------------------------------------------------------------|
| [Create embedded transactions](#creating-embedded-transactions) | <dy:SymbolTransactionFactory.createEmbedded>                                        |
| [Build the aggregate](#building-the-aggregate-transaction)      | <dy:SymbolTransactionFactory.create><br/><dy:SymbolFacade.hashEmbeddedTransactions> |
| [Sign the transaction](#signing-the-transaction)                | <dy:SymbolFacade.signTransaction>                                                   |
| [Verify and cosign](#verifying-and-cosigning)                   | <dy:SymbolFacade.cosignTransaction>                                                 |
| [Collect the cosignature](#collecting-the-cosignature)          | <dy:SymbolTransactionFactory.attachSignature>                                       |

## Next Steps

* **Batch from one account:** If all transactions share the same signer and no cosignatures are needed, see the
  [Batching Transactions](./transaction-batching.md) tutorial for a simpler flow.
* **Use bonded aggregates:** If parties cannot coordinate off-chain, see the
  [Creating a Complete Aggregate Transaction](./complete-aggregate.md) tutorial for a fully on-chain alternative.
* **Sponsor fees:** Let one account pay transaction fees on behalf of another following the
  [Paying Transaction Fees on Behalf of Another Account](./fee-sponsorship.md) tutorial.
