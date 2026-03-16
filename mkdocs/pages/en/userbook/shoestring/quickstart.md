# Running a Node

This tutorial shows how to create and start a Symbol node using the Shoestring tool.

The tutorial assumes that Shoestring and its dependencies are already installed.
See [Installing Shoestring](./install.md).

## Create a New Node

Deploying a node with Shoestring consists of five steps:

1. Create a working directory.
2. Generate network configuration files.
3. Configure the desired roles.
4. Create the node identity.
5. Create the node installation.

The following sections walk through these steps.

??? info "Using the Shoestring Wizard"

    Alternatively, you can use the interactive setup wizard provided by Shoestring by running:

    ```bash
    python3 -m shoestring wizard
    ```

    The wizard asks a series of questions and generates the same configuration files used in this tutorial.

    The resulting node behaves exactly the same as one created with the command-line steps shown below.

    If you use the wizard, then, after completing it, continue from the step [Start the Node](#start-the-node).

### 1. Create a Working Directory

Shoestring creates the node installation inside the current directory.

Create a directory for the node and move into it:

```bash
mkdir symbol-node
cd symbol-node
```

All commands in this tutorial assume they are executed from this directory.

### 2. Generate the Network Configuration

The first step is to generate the configuration files required for the node.

Run:

```bash
python3 -m shoestring init --package mainnet config.ini
```

This command downloads the configuration package for the selected network and creates the file `config.ini`.

For testnet nodes, use:

```bash
python3 -m shoestring init --package sai config.ini
```

### 3. Configure Node Roles

The node [roles](../../textbook/nodes.md#roles), called _features_ by Shoestring,
are defined in the `#!ini [node]` section of `config.ini`.

Open the file and locate the following section:

```ini
[node]
features =
```

Set the desired node features from the list: `PEER`, `API`, `HARVESTER`, and `VOTER`.
You can combine multiple features separating them with a vertical bar `|`, for example: `PEER | API`

For example, an API node can be configured with:

```ini
[node]
features = API
```

### 4. Create the Node Identity

Each node requires a cryptographic identity used for secure communication.

Generate the node key with OpenSSL:

```bash
openssl genpkey -algorithm ed25519 -out ca.key.pem
```

This creates a file named `ca.key.pem`, which **must be kept secure**.
It contains the private key used to generate the node certificates.

### 5. Create the Node Installation

Once the configuration and key files are ready, create the node installation.

Run:

```bash
python3 -m shoestring setup \
  --config ./config.ini \
  --package mainnet \
  --directory . \
  --ca-key-path ./ca.key.pem
```

This command performs several tasks:

* downloads the Symbol server software
* creates the node directory structure
* generates certificates
* prepares the Docker environment

All necessary configuration is now done and the node can be started.

## Start the Node

Start the node containers with Docker:

```bash
docker compose up -d
```

The `-d` option runs the containers in detached mode, so the node continues running even after closing the terminal.

To stop the node, run the following command from the same folder:

```bash
docker compose down
```

The node data remains on disk and the node can be started again with `docker compose up -d`.

## Verify the Node

Check that the node is running correctly:

```bash
python3 -m shoestring health --config config.ini --directory .
```

If the node started successfully, the command reports that the peer endpoint, REST API, and WebSocket services are
reachable.

## Next Steps

Once the node is running, additional tasks may be required:

* customizing node configuration
* upgrading the client
* renewing certificates
* migrating from symbol-bootstrap

See the following pages for details.
