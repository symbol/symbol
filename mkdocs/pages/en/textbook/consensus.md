# Consensus

Consensus
:   The process by which all <nodes:> in the network agree on the current state of the blockchain.

With consensus, the network preserves a single, consistent timeline of <blocks:> and their <transactions:>,
and therefore the balance and data associated with every <account:>.

Consensus provides two forms of agreement:

* **Sealing agreement**:
    each block correctly links to its predecessor, ensuring the immutability of the chain's history.

* **Content agreement**:
    all transactions in a block comply with the network's rules.
    For example, transferring tokens from an account requires a valid signature from its private key
    and a sufficient balance.

Blocks that violate either form of agreement are _invalid_ and ignored by well-behaved nodes.
Such blocks are not propagated through the network.

## Conflicts

In a decentralized network like Symbol, though, <nodes:> may temporarily become disconnected.
This can happen due to latency, connectivity issues, or changes in network topology.

During _network partitions_, disconnected groups of nodes may temporarily disagree on the most recent blocks,
even though they might all be valid.

As a result, more than one version of the blockchain may exist for a short time, a situation known as a _fork_.

Fork
:   State where two or more competing chains share a common history but differ in their latest blocks.

During a fork, for example, queries to different nodes might return different balances for the same account,
depending on whether the queried nodes have seen all the transactions that affect that account.

When connectivity is restored, nodes might encounter competing blocks for the same height, resulting in a conflict.

Forks might also occur naturally when two nodes produce a new block at the same time.

## Conflict Resolution

When a node becomes aware of a fork, Symbol resolves it using a deterministic rule:
the chain with the highest <chain score:>, based on cumulative difficulty, is considered the correct one.

Nodes on the lower-scoring fork need to _roll back_ any blocks that are no longer part of the
main chain and switch to the better one.

Rollback
:   The process of discarding one or more recently added blocks when a node switches to a better chain,
    typically after a fork is resolved.

Any transactions in the discarded blocks that are not already present in the main chain
return to the <unconfirmed pool:> and must be re-verified before they can be included in a block again.

Rollbacks on Symbol are usually shallow and rare, affecting only the most recent blocks.
However, to completely remove this risk, Symbol uses _finalization_.

## Finalization

Finalization
:   The process that makes blocks, and the transactions they contain, irreversible, eliminating the risk of
    <rollbacks:>.

Once a block is finalized, it is guaranteed to remain in the chain and will never be rolled back, even in the event of
a network partition or a <fork:>.

Finalization is carried out by eligible <voting nodes:>, which periodically cast votes weighted by their importance.
These votes determine the highest block that can be finalized, and by implication all preceding blocks as well.
Any block that receives at least two-thirds of the weighted votes is considered finalized.

The result is a _finalization point_: a checkpoint in the blockchain that all nodes agree to build upon.

Finalization gives applications a reliable foundation to work from.
For example, a wallet can consider a transaction to be fully confirmed once it is included in a finalized block.

Votes are weighted by account balance to prevent <Sybil attacks:>,
and only accounts holding at least 10'000 <XYM:> are eligible to participate.

In the absence of network partitions, blocks are typically finalized within 10 to 20 minutes,
depending on how long ago the last finalization point occurred.
