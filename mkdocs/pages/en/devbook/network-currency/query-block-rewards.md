---
title: Query Block Rewards
tutorial_level: beginner
---

# Querying Block Rewards

Each <block:> on Symbol generates a reward consisting of <inflation:> plus the <transaction:> fees collected in that
block.
This reward is then [distributed](../../textbook/harvesting.md#reward-distribution) among the <harvesting:|harvester>,
the node beneficiary, and the network sink account.

This tutorial shows how to query the reward for any block and break down its distribution among accounts using
<receipts:>.

## Prerequisites

Before you start, [set up your development environment](../start/setup.md).

This tutorial only reads data from the network. No <account:> or <XYM:> balance is required.

## Full Code

{% import 'tutorial.jinja2' as tutorial with context %}

{{ tutorial.code_full_tagged('devbook/network-currency/query_block_rewards', ['py', 'js']) }}

## Code Explanation

The code retrieves the block's signer key, beneficiary and network sink addresses, along with the inflation amount.
It then queries the block's harvest receipts and labels each recipient by comparing addresses, identifying the harvester
by elimination.
Finally, it derives the transaction fees by subtracting inflation from the total.

### Fetching Block Information

{{ tutorial.code_snippet_tagged('step-1') }}

The snippet retrieves the block header using the `NODE_URL` and `BLOCK_HEIGHT` environment variables to select the API
node and the target block.
If not set, they default to the reference testnet node and block `3222290`.

The <get:/blocks/{height}> endpoint returns the block header, which includes the `signerPublicKey` and the
`beneficiaryAddress` designated by the node operator.
The beneficiary address is needed later to identify beneficiary receipts.

### Fetching the Network Sink Address

{{ tutorial.code_snippet_tagged('step-2') }}

The `harvestNetworkFeeSinkAddress` is fetched from <get:/network/properties> and needed later to identify sink receipts.
Since the property is in base32 format and receipt addresses are hex-encoded, the SDK's `Address` class converts it
to hex for comparison.

### Fetching the Inflation Reward

{{ tutorial.code_snippet_tagged('step-3') }}

The <get:/network/inflation/at/{height}> endpoint returns the inflation reward amount for the given block height.
This value is in <divisibility:|atomic> units, so for XYM (divisibility 6), `113474978` atomic units represent
`113.474978` whole units.

The inflation schedule is defined in the network configuration and decreases over time.

### Querying the Reward Distribution

{{ tutorial.code_snippet_tagged('step-4') }}

To see how the total reward (inflation + transaction fees) was distributed, the code queries the
<get:/statements/transaction> endpoint filtered by
[`receiptType=8515`](../reference/rest/symbol.md#model/ReceiptTypeEnum) (`Harvest_Fee`), which returns the exact amount
each participant received for harvesting the block.

!!! note "Receipt type filter"
    The `receiptType` parameter filters statements that contain at least one receipt of that type, but each statement
    may also include other receipt types.
    The code skips non-harvest receipts within the same statement.

Each receipt's `targetAddress` is compared against the beneficiary and sink addresses to label each recipient.
The remaining address is the harvester.

If the harvester and beneficiary are the same account, the network skips the beneficiary share and the harvester
receives the full remainder.
In that case only two receipts are created (harvester + sink) instead of three.

!!! info "Why not use the block's `signerPublicKey`?"
    Harvesters typically sign blocks with a [remote key](../../textbook/harvesting.md) rather than their main key.
    In that case, the block's `signerPublicKey` does not correspond to the harvester's main address.
    The remote key can be resolved by querying <get:/accounts/{accountId}> with the signer key and following its
    `supplementalPublicKeys.linked.publicKey` to the main account.
    However, this link reflects the current account state and may have changed since the block was harvested.
    Receipts are the reliable source.

The sum of all `Harvest_Fee` receipts equals the total block reward (inflation + transaction fees).

### Calculating the Fee Breakdown

{{ tutorial.code_snippet_tagged('step-5') }}

Finally, subtracting the inflation amount from the total block reward gives the transaction fees collected in the block.
All values are converted from atomic units to whole units for display.

Alternatively, the fee total can be calculated by summing each transaction's effective fee, which equals its size in
bytes multiplied by the block header's `feeMultiplier`.

## Output

The following output shows a typical run querying the rewards for block 3,222,290:

```text linenums="1" hl_lines="3-5 9-12 15-17"
--8<-- 'devbook/network-currency/query_block_rewards.log'
```

Some highlights from the output:

* **Addresses** (lines 3-5): The signer address is derived from the block's `signerPublicKey`.
    The beneficiary is set by the node operator, and the network sink is a fixed system account defined in the network
    configuration.

* **Reward distribution** (lines 9-12): Each participant's share of the total block reward.
    The harvester is identified by elimination, as any receipt target that is neither the sink nor the beneficiary.
    Notice the signer address (line 3) differs from the harvester address (line 10) because the harvester uses a
    [remote key](../../textbook/harvesting.md) to sign blocks.

* **Summary** (lines 15-17): The total block reward is the sum of all `Harvest_Fee` receipts.
    The inflation portion comes from the network configuration, while the transaction fees (0.032800 XYM) are the
    difference between the total reward and inflation.

## Conclusion

This tutorial showed how to:

| Step                                                             | Related documentation                |
| ---------------------------------------------------------------  | ------------------------------------ |
| [Fetch block information](#fetching-block-information)           | <get:/blocks/{height}>               |
| [Fetch network sink address](#fetching-the-network-sink-address) | <get:/network/properties>            |
| [Fetch inflation reward](#fetching-the-inflation-reward)         | <get:/network/inflation/at/{height}> |
| [Query reward distribution](#querying-the-reward-distribution)   | <get:/statements/transaction>        |
