# Harvesting

Harvesting
:   The process by which Symbol adds new <blocks:> to the chain and distributes rewards to participating <accounts:>.
    It plays a similar role to **mining** in <PoW:> or **staking** in <PoS:>.

Each new block is created by a single, random <node:> and signed by one of the node's _harvester accounts_.
This account can either be the node's main account or any of its <delegators:>.

The fees from the <transactions:> included in the block, along with a portion of <inflation:>,
are distributed to the harvester account and to other accounts designated by the node operator,
which may or may not include the node's main account.

Unlike mining in <PoW:>, harvesting does not require specialized hardware.
Participation is open to any account holding at least 10,000 <XYM:> and connected to a node,
either directly or through delegation.
This balance contributes to the account's <importance:> score, which determines how often it can harvest a block.

## Harvesting Process

Symbol does not use an explicit selection process to determine which node will harvest the next block.
Instead, every node independently attempts to produce a new block.

First, a random number called the _hit_ is computed from the hash of the previous block.

The node then iterates over its main account and all linked <delegator:> accounts,
calculating a _target_ value for each one based primarily on its <importance:>.
Accounts with higher importance produce higher target values.

If the _hit_ is lower than the _target_ for any of these accounts,
the node assembles a candidate block by collecting transactions from the <unconfirmed pool:>
and assigning the harvesting rewards to the configured beneficiaries.

The completed block is then announced to the rest of the network and other nodes then verify it, ensuring:

* The block signature comes from the claimed harvester.
* The transactions are valid.
* The target and hit values were correctly calculated.
* The hit is indeed lower than the target.

If any of these checks fail, other nodes simply ignore the new block.
The <consensus:> mechanism makes sure that the node eventually adopts a block that the rest of the network agrees on.

If the block is valid, it is accepted by other nodes that include it in their copies of the chain.
This process is repeated at the next block height.

!!! info "Simultaneous Block Creation"
    Note that no special measures are in place to prevent multiple nodes from generating blocks at the same height.
    When this occurs, the network may temporarily <fork:> as different nodes adopt different blocks for the same
    position in the chain.

    The <consensus:> mechanism resolves these conflicts as nodes become aware of the competing blocks.

??? abstract "Target and Hit Calculation"

    * The **target** is calculated independently by each node and reflects its likelihood of harvesting the next block
        using a specific account.
        It depends on three factors:

        * The account's <importance:> score: more active or better-funded accounts will harvest more often.
        * The network-wide **difficulty**, which adjusts dynamically based on recent block production times,
            to maintain a constant rate.
        * The **time** elapsed since the last block: longer delays increase the chance of a new block being produced.

    * The **hit** is a random number derived from the new block's content and the account's <VRF key:>,
        making it unpredictable.

        Without the VRF key, an attacker could predict which account is next in line and attempt to censor its
        associated node, for example, by launching a <DoS:> attack.

    For the block to be valid, the node's target must be **greater than** its hit.
    A higher importance or a longer delay increases the target, making the condition easier to satisfy.

## Harvesting Methods

Node owners can participate in harvesting by enabling [local](#local-harvesting) or [remote](#remote-harvesting) harvesting,
depending on their preferred balance between simplicity and security.
Accounts that do not operate a node but meet the balance requirements can still harvest by linking to a node through
[delegated harvesting](#delegated-harvesting), sharing in its rewards.

### Local Harvesting

Local Harvesting
:   A type of <harvesting:> where the rewards are sent directly to the harvester account.
    The node signs produced blocks using the operator's <main key:>, which must be stored on the machine.

!!! warning
    The harvester account must hold a significant balance to maintain a high <importance:> score.
    Storing its <private key:> on a machine that is permanently online puts the entire balance at risk
    in case of unauthorized access.

While local harvesting offers a straightforward setup, these security risks make it unsuitable for public nodes.
Most operators instead prefer remote harvesting.

### Remote Harvesting

Remote Harvesting
:   A type of <harvesting:> that delegates block signing to a separate _proxy_ account,
    while the node's <importance:> score and rewards remain tied to the operator's <main key:>.

The proxy account holds no funds and exists only to sign blocks on behalf of the harvester's main account.
Because its <private key:> is stored in the node's configuration files, hosted on a permanently-online machine,
it is designed to be expendable.

The main account still determines the node's importance and receives all block rewards.
However, its key remains offline, safe from compromise.
For simplicity, the main account is still called the harvester account, even though blocks are signed by the proxy.

This separation of duties offers strong protection for the harvester's funds
and makes remote harvesting the preferred option for most operators.

### Delegated Harvesting

Delegated Harvesting
:   A form of <harvesting:> that allows eligible accounts not operating a node to delegate harvesting duties to a
    third-party node.
    The account's <importance:> score is used, and any harvested fees are shared between the account and the node's
    configured beneficiary.

Such an account is called a _delegator_, or _delegated harvester_.

Delegator
:   An account that <delegated harvesting:|delegates> harvesting to a third-party node while retaining its <importance:>
    and receiving a share of the harvested rewards.
    Also called a _delegated harvester_.

Although the node performs the work, the delegator is still considered the harvester.
This arrangement benefits both parties: the account earns rewards without running a node,
and the node increases its block production (and collected fees) without relying solely on its own importance.

Delegated harvesting uses the same proxy-based setup as remote harvesting.
In addition, the delegator signs a <transfer transaction:> with an encrypted message that requests the node to
harvest on its behalf.

Whether the node grants the request depends on its policy and any competing requests it may have received.

As with remote harvesting, block signing is performed by an account other than the delegator,
so its <private key:> never needs to leave secure storage.

## Reward Distribution

When a block is harvested, rewards are distributed among several participants.
These rewards consist of:

* The fees paid by the <transactions:> included in the block.
* An additional amount of <inflation:>.

The total reward is divided among the following parties:

```dot
digraph RewardDistribution {
    node [shape=box]
    edge [penwidth=1.5]

    BlockRewards [label="Block Rewards\n(Fees + Inflation)"]
    Harvester [label="Harvester" URL="#harvester"]
    Sink [label="Network sink account" URL="#sink"]
    Beneficiary [label="Optional\nnode-designated\nbeneficiary account" URL="#beneficiary"]

    T1 [shape=point width=0 height=0]
    T2 [shape=point width=0 height=0]

    BlockRewards -> T1:s [dir=none];
    T1 -> T2:s [label=" 95%" weight=2 dir=none];
    T1 -> Sink [label="5%" minlen=3];
    T2 -> Harvester [label="Remaining" weight=2];
    T2 -> Beneficiary [label="25%" minlen=3 style=dashed];

    {rank=same; T1; Sink;}
    {rank=same; T2; Beneficiary;}
}
```

* **The network sink account**:
    {id="sink"}

    A system account that receives a fixed 5% of the reward.
    This amount can be used to fund network-wide reward programs.

* **The node-designated beneficiary**:
    {id="beneficiary"}

    An optional account, chosen by the node owner,  that receives a fixed 25% of the reward
    (after subtracting the network sink amount).

    Node owners may retain this portion for themselves or redistribute it through their own reward programs,
    for example, by sharing it with their delegators.

* **The harvester**:
    {id="harvester"}

    The account that created the block.
    It can be the node's <remote harvesting:> account, or any of its <delegators:>.

!!! example
    For example, assuming 100 XYM of block rewards (fees + inflation) and that a beneficiary account is configured:

    <div class="centered" markdown>

    | Recipient                | Calculation             |      Amount |
    | ------------------------ | ----------------------- | ----------: |
    | **Network sink account** | 5% of 100 XYM           |       5 XYM |
    | **Beneficiary account**  | 25% of remaining 95 XYM |   23.75 XYM |
    | **Harvester account**    | Remaining XYM           |   71.25 XYM |
    | **Total**                |                         | **100 XYM** |

    </div>
