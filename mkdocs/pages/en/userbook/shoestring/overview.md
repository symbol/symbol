# Shoestring Overview

Shoestring
:   A command-line tool used to deploy and manage <nodes:> in the Symbol network.

It replaces the older `symbol-bootstrap` tool, which is no longer maintained.
Shoestring performs the same core tasks, but with a simpler architecture and fewer dependencies.

Using Shoestring, node operators can:

* generate node configuration files
* deploy new nodes
* manage node certificates and voting keys
* upgrade the Symbol client
* reset or migrate existing nodes

Shoestring does not run the node itself.
Instead, it prepares a [Docker](https://www.docker.com) environment that contains the
[Symbol node components](../../textbook/nodes.md#node-structure).

## Why Shoestring Exists

Earlier Symbol deployments relied on `symbol-bootstrap`.
That tool depended on [Node.js](https://nodejs.org/) and included complex logic for configuration generation and
orchestration.

Shoestring simplifies node deployment by:

* using Python instead of Node.js
* separating configuration generation from runtime execution
* delegating runtime orchestration to Docker

This design reduces dependencies and makes node operations easier to maintain over time.

## How Shoestring Works

Shoestring prepares the files and configuration required to run a Symbol node.

The typical workflow is:

1. Generate network configuration files.
2. Customize node settings if required.
3. Run the Shoestring setup command.
4. Start the Docker containers that run the node.

The running node consists of several components inside Docker containers, including the
[Catapult server](../../textbook/nodes.md#catapult) and the
[REST Gateway](../../textbook/nodes.md#octicons-terminal-24-rest-gateway).

Shoestring also provides commands that help maintain a running node, such as upgrading the client,
renewing certificates, or checking node health.

## Docker Requirement

Shoestring deploys Symbol nodes using Docker containers.

Running a node directly from the Symbol server binaries is technically possible, but this approach
requires manual configuration and process management.
For this reason, Docker-based deployments are recommended for most operators.

All tutorials in this section assume Docker is installed and available on the system.

## Typical Workflow

Operating a Symbol node with Shoestring usually follows these steps:

1. [Install Shoestring](./install.md).
2. Generate configuration files for the desired network.
3. Create the node installation.
4. Start the node containers.
5. Maintain the node over time.

The following pages describe each of these steps in more detail.
