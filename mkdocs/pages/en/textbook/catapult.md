# Catapult

!!! image inline end ""

    ![Catapult](site:/assets/images/catapult.png){.off-glb .invertible}

Catapult
:   The reference implementation for the software client at the core of every Symbol <node:>.
    It verifies <transactions:> and <blocks:>, runs the <consensus:> algorithm, creates new blocks,
    and propagates the changes through the network.

Symbol refers to the ecosystem and public network, while Catapult is the software that defines how that network behaves.
Every node in Symbol runs Catapult at its core, executing the same deterministic logic to reach agreement on the blockchain state.

Catapult was designed as the successor to the original NEM ([NIS1](https://docs.nem.io)) client,
addressing the limitations of its monolithic design.
It introduces a new modular architecture written in C++, emphasizing performance, flexibility, and extensibility.
Where NIS1 bundled all blockchain logic in a single Java codebase, Catapult separates each feature, such as mosaics,
namespaces, or multisignature accounts, into independent plugins.

This architecture allows the protocol to evolve without changing the underlying consensus engine,
enabling both public and private deployments to share the same foundation.
Catapult thus serves as the technical backbone of Symbol: a general-purpose blockchain engine capable of
powering multiple networks, each defined by its configuration and plugin set.

## Design Principles

Catapult was built with a focus on modularity, performance, and reliability.
Its architecture allows networks to evolve without altering the consensus core and ensures that all nodes process data
efficiently.

The main principles guiding its design are the following:

* **Modularity**: Every feature, from <mosaics:> to <multisignature accounts:>, is implemented as a plugin that can be
    added or updated independently.
* **Performance**: Core logic is written in C++ and optimized for concurrency and low-latency processing.
* **Extensibility**: New transaction types or features can be introduced without rewriting the consensus logic.
* **Configurability**: Chain parameters such as block time, inflation, or importance weighting are defined in
    configuration files.
* **Separation of concerns**: The Catapult engine focuses only on protocol execution, while the
    [REST gateway](./nodes.md#rest-gateway), <SDKs:>, and [databases](./nodes.md#node-structure)
    handle external access and data indexing.

## Role Within the Node

Catapult is the core process within each Symbol node.
It maintains the blockchain state, validates transactions, and applies protocol rules.
Other components interact with it to expose network data or accept new transactions,
but Catapult itself remains the sole authority on state changes.

Catapult interacts with the following elements:

* [REST gateway](./nodes.md#rest-gateway):
    Provides HTTP access to blockchain data and transaction submission.

* [Databases](./nodes.md#node-structure):
    Store the blockchain state for fast queries.

* <Peer nodes:>:
    Catapult interacts with other nodes implementing the Symbol protocol to partake in <consensus:>.

For most applications, direct communication with Catapult is unnecessary.
They use the REST gateway instead, either directly or through the <SDK:>.

## Plugin Model

Catapult was designed around a plugin and extension architecture rather than a Turing-complete smart contract system.
This approach limits the operations that can be performed on the blockchain to a well-defined set, reducing the attack
surface and making it easier to optimize performance.

Two different extensibility mechanisms are provided:

* **Plugins** define the transaction types supported by a network.

    Every node in the same network must load the exact same set of transaction plugins and process them in the same way,
    so that all nodes can agree on the global blockchain state.

    These plugins are listed in the network configuration, and any change to this set requires coordination among
    all nodes, since differences would result in a <fork:>.

    All built-in Symbol transaction types—such as <transfer transaction:|transfers>, <mosaics:>, <namespaces:>, or
    <aggregate transactions:> are implemented as plugins.

* **Extensions**, on the other hand, are optional components that can vary between nodes.

    They provide additional capabilities that do not affect consensus, such as storing data in a database or publishing
    events to external systems.

    Extensions are configured at the node level and allow operators to customize their node's behavior without impacting
    network compatibility.

## Extensibility and Configuration

Catapult can power a variety of networks, from Symbol's public blockchain to private or consortium deployments.
Its behavior is defined almost entirely through configuration files, which specify both network parameters and
node-level options.

* **Network configuration** determines the behavior shared by all nodes in a chain.

    It defines parameters such as block time, importance weighting, harvesting rewards, and the set of
    transaction plugins that the network supports.

    All nodes must use the same network configuration to remain compatible.

* **Node configuration** defines optional behavior that can differ between nodes.

    This includes which extensions to load, database or logging options, and connections to external services.
    Because these settings do not affect consensus, each node can customize them independently.

This layered approach enables Symbol to maintain a consistent protocol across the network while still allowing
individual nodes to specialize, optimize, or integrate with external systems.

!!! note "Symbol network configuration"
    The public Symbol network (mainnet) is one particular deployment of Catapult.
    Its configuration files, including network parameters and plugin definitions,
    are available in the [symbol/networks](https://github.com/symbol/networks/tree/mainnet/resources) repository.

## Evolution and Legacy

Catapult was developed as the next-generation implementation of the NEM blockchain, originally known as _NEM 2.0_.
Its goal was to overcome the structural limitations of the first NEM client ([NIS1](https://docs.nem.io)),
which had a monolithic Java-based architecture that made large-scale upgrades difficult.

The new implementation, written in C++, introduced a modular design that separates consensus, transaction logic, and
extensions into independent components.
This separation improved performance, scalability, and maintainability, allowing the same core engine to support both
public and private networks.

Catapult also introduced major protocol-level advancements, including <aggregate transactions:>,
<multisignature accounts:> with multiple layers, and a more efficient system for managing <mosaics:> and <namespaces:>.
These features laid the foundation for Symbol, the first public network to deploy the Catapult engine in production.
