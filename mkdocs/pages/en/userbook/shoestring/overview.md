# Shoestring Overview

Shoestring
:   A command-line tool used to deploy and manage <nodes:> in the Symbol network.

Using Shoestring, node operators can:

* Deploy new nodes.
* Migrate existing nodes.
* Keep the node software up to date.
* Manage node certificates and voting keys.

Shoestring does not run the node itself.
Instead, it prepares a [Docker](https://www.docker.com) environment that contains the required
[Symbol node components](../../textbook/nodes.md#node-structure).

!!! warning

    Operating Shoestring and Symbol nodes in general requires familiarity with the terminal and command-line tools
    of your operating system.
    These guides assume basic command-line usage.

## Why Shoestring Exists

Earlier Symbol deployments relied on `symbol-bootstrap`.
That tool depended on [Node.js](https://nodejs.org/) and included complex logic for configuration generation and
orchestration.

Shoestring simplifies node deployment by:

* using Python instead of Node.js because it's typically included in most operating systems
* separating configuration generation from runtime execution

This design reduces dependencies and makes node operations easier to maintain over time.

## How Shoestring Works

Shoestring prepares the configuration files required to run a Symbol node with the desired
[roles](../../textbook/nodes.md#roles).

The typical workflow is:

1. Generate network configuration files.
2. Customize node settings if required.
3. Run the Shoestring setup command.
4. Start the Docker containers that run the node.

The running node consists of several components inside Docker containers, including the
[Catapult client](../../textbook/nodes.md#catapult) and the optional
[REST Gateway](../../textbook/nodes.md#rest-gateway).

Shoestring also provides commands that help maintain a running node, such as upgrading the client,
renewing certificates, or checking node health.

## Docker Requirement

Running a node directly from the Symbol server binaries is technically possible, but this approach
requires manual configuration and process management.
For this reason, Docker-based deployments are recommended for most operators.

All guides in this section assume Docker is installed and available on the system.
