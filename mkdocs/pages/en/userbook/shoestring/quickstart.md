# Creating and Running a Node

This tutorial shows how to create and start a Symbol node using the <Shoestring:> tool.

The tutorial assumes that Shoestring and its dependencies are already installed.
See [Installing Shoestring](./install.md).

## Create a New Node

Deploying a node with Shoestring requires a few steps, described below.

??? info "Using the Shoestring Wizard"

    Alternatively, you can use the interactive setup wizard provided by Shoestring:

    ```bash
    python3 -m shoestring.wizard
    ```

    The wizard asks a series of questions and generates the same configuration files used in this tutorial.

    The resulting node behaves exactly the same as one created with the command-line steps shown below.

    After completing the wizard, continue from [Start the Node](#start-the-node)
    if the node has not been started automatically.

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

### 3. Configure Shoestring

The `config.ini` file generated in the previous step already contains most required values.
Only a few values need to be customized for your node, and they all reside in the `#!ini [node]` section:

* The node [roles](../../textbook/nodes.md#roles), called _features_ by Shoestring.

    Set the desired node features from the list: `PEER`, `API`, `HARVESTER`, and `VOTER`.
    You can combine multiple features separating them with a vertical bar `|`, for example:

    ```ini title="config.ini"
    [node]
    features = PEER | API
    ```

* The `caCommonName` and `nodeCommonName` are used as Common Names (CN) in the certificates generated for the node.
    They do not affect node behavior, but must not be empty for certificate generation to succeed.

    For example:

    ```ini title="config.ini"
    [node]
    caCommonName = My Symbol Node CA
    nodeCommonName = My Symbol Node
    ```

### 4. Configure the Node

An additional configuration file can be used to provide node-specific settings.
Without it, the node will not be usable.

Create a new file called `overrides.ini` with the following content:

```ini title="overrides.ini"
[node.localnode]
host = 127.0.0.1
friendlyName = My Symbol Node
```

Use the public IP address or hostname of your node as seen by the rest of the network.
If you use an unreachable host (like `127.0.0.1`), your node will connect to the network but other nodes will not
interact with it.

Use a human-readable name that will appear in node lists.

### 5. Create the Node Identity

Each node requires a cryptographic identity used for secure communication.

Generate the node key with OpenSSL:

```bash
openssl genpkey -algorithm ed25519 -out ca.key.pem
```

This creates a file named `ca.key.pem`, which **must be kept secure**.
It contains the private key used to generate the node certificates.

### 6. Create the Node Installation

Once the configuration and key files are ready, create the node installation.

Run:

```bash
python3 -m shoestring setup \
  --config ./config.ini \
  --overrides ./overrides.ini \
  --package mainnet \
  --directory . \
  --ca-key-path ./ca.key.pem
```

This command performs several tasks:

* downloads the Symbol server software
* creates the node directory structure
* generates certificates
* prepares the Docker environment

The node is now configured and ready to be started.

## Start the Node

Start the node containers with Docker:

```bash
docker compose up -d
```

The `-d` option runs the containers in detached mode, so the node continues running even after closing the terminal.

!!! warning "Docker permissions"

    If you see an error similar to:

    ```text
    permission denied while trying to connect to the Docker daemon socket
    ```

    your user does not have permission to run Docker commands.

    On Linux, you can add your user to the `docker` group with:

    ```bash
    sudo usermod -aG docker $USER
    ```

    Then log out and log back in, and run the command again.

To stop the node, run from the same folder:

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
reachable:

```txt
     ...    | running health agent for peer certificate
      i     | ca certificate not near expiry (7299 day(s))
      i     | node certificate not near expiry (374 day(s))
keys/cert/ca.crt.pem: OK
keys/cert/node.crt.pem: OK
     ...    | running health agent for peer API
      i     | peer API accessible, height = 226081
     ...    | running health agent for REST API
      i     | REST API accessible, height = 226081
     ...    | running health agent for REST websockets
      i     | websocket connected to ws://127.0.0.1:3000/ws, subscribing and waiting for block
      i     | websocket received a block with height 226082
```

While the node is synchronizing with the network, the reported chain height is lower than the actual height.

## Next Steps

Once the node is running, additional tasks may be needed:

* customizing node configuration
* upgrading the client
* renewing certificates
* migrating from symbol-bootstrap

See the following pages for details.
