---
title: Cross-Chain Swap
---

# Cross-Chain Swap Between Symbol and Ethereum

Two parties, Alice and Bob, want to exchange 0.01 ETH (on Ethereum) for 1 <XYM:> (on Symbol) without trusting each other
or using an intermediary.
Since these tokens exist on two separate blockchains, a direct transfer is not possible.

This tutorial shows how to perform this <cross-chain swap:> using an
[HTLC](../../textbook/cross-chain-swaps.md#protocol) smart contract on Ethereum and Symbol's native
<ser:SecretLockTransactionV1> and <ser:SecretProofTransactionV1>.

!!! info "Supported chains"
    This tutorial uses Ethereum as the counterpart chain, but Symbol's secret lock mechanism works with any
    blockchain that supports HTLCs.

## Prerequisites

Before you start, make sure to:

* Set up your development environment.
    See [Setting Up a Development Environment](../start/setup.md).
* Create two Symbol <accounts:>, one for Alice and one for Bob.
    See [Creating an Account from a Private Key](../accounts/create-from-private-key.md).
* Obtain XYM for Bob's account to pay for the secret lock transaction fee and locked amount.
    See [Getting Testnet Funds from the Faucet](../accounts/testnet-faucet.md).
* Create two Ethereum accounts, one for Alice and one for Bob.
    You can use [Foundry](https://book.getfoundry.sh/getting-started/installation)'s `cast wallet new` command or any
    Ethereum wallet such as MetaMask.
* Have Sepolia testnet ETH in both Ethereum accounts to pay for gas fees and enough in Alice's account to fund the HTLC.
    Sepolia ETH can be obtained from the
    [Google Cloud faucet](https://cloud.google.com/application/web3/faucet/ethereum/sepolia) or any other Ethereum
    testnet faucet.

* Install the Ethereum library for your language:

    === ":simple-python: Python"

        ```bash
        pip install web3
        ```

    === ":simple-javascript: JavaScript"

        ```bash
        npm install ethers
        ```

For background on the HTLC protocol, timing constraints, and limitations, see the
[Cross-Chain Swaps](../../textbook/cross-chain-swaps.md) concept page.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full('devbook/transactions/cross-chain-swap', ['py', 'js']) }}

In practice, both parties (Alice and Bob) would each run their own part on different machines.
This tutorial combines both sides in a single script for simplicity.

The code defines helper functions to fetch the network time and fees, announce transactions, and poll for confirmation,
following the same patterns described in the [Transfer](./transfer.md) tutorial.

This tutorial does not wait for transaction [finality](../../textbook/cross-chain-swaps.md#finality) between steps,
which a production implementation must do to prevent rollback-related risks.

## Ethereum HTLC Contract

This tutorial uses a sample HTLC contract deployed on Ethereum as the counterpart to Symbol's secret lock.
The contract source is available in the
[hashed-timelock-contract-ethereum](https://github.com/gimre-xymcity/hashed-timelock-contract-ethereum)
repository.

!!! warning "Educational use only"
    Any contract used in production must carefully calibrate lock and contract expiry times, as timing is critical for
    the security of both parties.

The contract provides three key methods:

* `newContract(address receiver, bytes32 hashlock, uint timelock)`: Creates a new HTLC with a recipient, hashlock, and
    a Unix timestamp as timelock.
    Comparable to Symbol's <ser:SecretLockTransactionV1>.
* `withdraw(bytes32 contractId, bytes proof)`: Allows the recipient to claim funds by providing the proof that matches
    the hashlock.
    Comparable to Symbol's <ser:SecretProofTransactionV1>.
* `refund(bytes32 contractId)`: Returns funds to the creator after the timelock expires.
    In Symbol, refunds happen automatically when a secret lock expires.

The contract has been deployed on the Sepolia testnet at address `0xd58e030bd21c7788897aE5Ea845DaBA936e91D2B`.

## Code Explanation

### Setting Up Accounts

{{ tutorial.code_snippet(['py:154:182', 'js:124:156']) }}

Both parties need accounts on both chains.
Alice uses her Ethereum account to lock ETH and her Symbol account to claim XYM.
Bob uses his Symbol account to lock XYM and his Ethereum account to withdraw ETH.

The `ALICE_PRIVATE_KEY` and `BOB_PRIVATE_KEY` environment variables set the Symbol keys, while
`ALICE_ETH_PRIVATE_KEY` and `BOB_ETH_PRIVATE_KEY` set the Ethereum keys.
Although pre-funded test keys are provided as defaults for convenience, they are not maintained and may run out of
funds.

### Alice: Generating the Proof and Hashlock

{{ tutorial.code_snippet(['py:186:193', 'js:160:167']) }}

Alice generates a random 32-byte value as the **proof**.
She then hashes it using double SHA-256 to produce the **hashlock**.

The double SHA-256 algorithm is chosen because it is supported by both Symbol (as `hash_256`) and the Ethereum HTLC
contract.
Using the same algorithm on both chains is essential for the swap to work.

!!! info "Other hash algorithms"
    Symbol supports other hash algorithms for secret locks.
    See <ser:LockHashAlgorithm> for all available values.

### Alice: Locking ETH on Ethereum

{{ tutorial.code_snippet(['py:196:219', 'js:170:192']) }}

Alice calls `newContract` on the Ethereum HTLC contract, locking 0.01 ETH for Bob:

* **Receiver:** Bob's Ethereum address.
* **Hashlock:** The double SHA-256 hash of the proof. Only Alice knows the proof at this point.
* **Timelock:** A Unix timestamp 72 hours in the future, after which Alice can reclaim the ETH if Bob does not complete
    the swap.
* **Value:** 0.01 ETH sent along with the transaction.

The transaction receipt contains a `LogHTLCNew` event with a `contractId` that identifies this HTLC.
Bob will need this `contractId` later to withdraw the ETH.

### Bob: Creating a Secret Lock on Symbol

{{ tutorial.code_snippet(['py:222:246', 'js:194:222']) }}

Bob first queries the Ethereum HTLC contract using `getContract` to retrieve the hashlock that Alice used.

!!! note "Verify before locking"
    Bob should verify the full contract details (amount, recipient, timelock) before locking his own funds.
    This tutorial only reads the hashlock for simplicity.

Bob then creates a <ser:SecretLockTransactionV1> on Symbol, locking 1 XYM for Alice, using the **same hashlock**:

* **Recipient:** Alice's Symbol address.
* **Mosaic:** 1 XYM (expressed as `1_000000` atomic units with divisibility 6).
* **Duration:** 5760 blocks (~48 hours at 30-second block times).

    !!! warning "Timelock ordering"
        This duration must be **shorter** than Alice's 72-hour Ethereum timelock.
        Otherwise, Alice could refund her ETH and still claim Bob's XYM.
        The gap between the two must be **large** enough: is the safety margin that guarantees Bob can still withdraw
        on Ethereum even if Alice reveals the proof at the last moment.
        See [Timing Constraints](../../textbook/cross-chain-swaps.md).

* **Hashlock (`secret` field):** The hashlock retrieved from the Ethereum contract.
* **Hash algorithm:** `hash_256` (double SHA-256), must match the algorithm used in the counterpart chain's HTLC.

### Alice: Claiming XYM on Symbol

{{ tutorial.code_snippet(['py:263:275', 'js:239:254']) }}

Once Bob's secret lock is confirmed, Alice can claim the locked XYM by revealing the proof.
She creates a <ser:SecretProofTransactionV1> with:

* **Recipient:** Alice's own Symbol address (the same address set in Bob's secret lock).
* **Hashlock (`secret` field):** The same hashlock used in the secret lock.
* **Hash algorithm:** `hash_256` (must match the secret lock).
* **Proof:** The original random bytes that Alice generated.

Once this transaction is announced and confirmed, the proof is **publicly visible** on the Symbol blockchain.
Bob (or anyone) can read it from the transaction data.

### Bob: Withdrawing ETH on Ethereum

{{ tutorial.code_snippet(['py:292:316', 'js:272:288']) }}

After Alice's secret proof is confirmed, Bob retrieves the proof from the confirmed transaction using the
<get:/transactions/confirmed/{transactionId}> endpoint and calls `withdraw` on the Ethereum HTLC contract with two
arguments:

* **Contract ID:** The HTLC identifier from the `LogHTLCNew` event emitted when Alice locked the ETH.
* **Proof:** The proof Alice revealed on Symbol.

!!! warning "Withdrawal deadline"
    Bob must complete this before Alice's Ethereum timelock expires.
    After that, Alice can call `refund` on the Ethereum contract and reclaim her ETH.

Once the Ethereum transaction is confirmed, the swap is complete: Alice has 1 XYM and Bob has 0.01 ETH.

## Output

The output shown below corresponds to a typical run of the program.

```text linenums="1" hl_lines="9 10 16 19 50 78 81 82"
--8<-- 'devbook/transactions/cross-chain-swap.log'
```

Key points in the output:

* **Lines 9-10:** Alice generates the proof and hashlock. The proof must remain secret until Alice reveals it.
* **Line 16:** The HTLC contract ID identifies Alice's Ethereum lock. Bob uses this to query the hashlock and later
    to withdraw.
* **Line 19:** Bob retrieves the hashlock from the Ethereum contract using `getContract`.
* **Line 50:** Bob's Symbol secret lock is confirmed. Alice can now claim the XYM.
* **Line 78:** Alice's secret proof is confirmed, revealing the proof on the Symbol blockchain.
* **Lines 81-82:** Bob retrieves the proof from the confirmed Symbol transaction, then uses it to withdraw on
    Ethereum.

You can verify the transactions on each network's block explorer using the hashes printed in the output:

* **Ethereum:** [Sepolia Etherscan](https://sepolia.etherscan.io/) for the lock and withdraw transactions.
* **Symbol:** [Symbol Testnet Explorer](https://testnet.symbol.fyi/) for the secret lock and secret proof transactions.

## Conclusion

This tutorial showed how to:

| Step                                                                         | Related documentation          |
| ---------------------------------------------------------------------------- | ------------------------------ |
| [Generate a proof and hashlock](#alice-generating-the-proof-and-hashlock)    | <ser:LockHashAlgorithm>        |
| [Lock ETH on Ethereum](#alice-locking-eth-on-ethereum)                       | Ethereum HTLC contract         |
| [Create a secret lock on Symbol](#bob-creating-a-secret-lock-on-symbol)      | <ser:SecretLockTransactionV1>  |
| [Reveal the proof on Symbol](#alice-claiming-xym-on-symbol)                  | <ser:SecretProofTransactionV1> |
| [Withdraw ETH on Ethereum](#bob-withdrawing-eth-on-ethereum)                 | Ethereum HTLC contract         |

## Next Steps

This tutorial is a simplified example.
Before using cross-chain swaps in production, review the
[Timing Constraints](../../textbook/cross-chain-swaps.md#timing-constraints) in the textbook.
