# Creating and Running a Node

This tutorial shows how to deploy and start a new Symbol <node:> using the <Shoestring:> tool.

The tutorial assumes that Shoestring and its dependencies have already been [installed](./install.md).

If you already have a node created with the deprecated Bootstrap tool and wish to reuse its
configuration or keys, pay special attention to the Bootstrap notes throughout this guide.

## Create a New Node

Deploying a node with Shoestring involves the following steps.

??? info "Using the Shoestring Wizard"

    Alternatively, you can use the interactive setup wizard provided by Shoestring:

    ```bash
    python3 -m shoestring.wizard
    ```

    The wizard asks a series of questions and generates the same configuration files created in this tutorial,
    but offers less customization options.

    The resulting node behaves exactly like one created with the command-line steps below.

    After completing the wizard, continue from [Start the Node](#start-the-node)
    if the node has not been started automatically.

### 1. Create a Working Directory

Shoestring creates the node files inside the current directory by default.

Create a directory for the node and move into it:

```bash
mkdir symbol-node
cd symbol-node
```

All commands in this tutorial assume they are run from this directory.

### 2. Generate the Network Configuration

The first step is to retrieve the default configuration files for the target network (<mainnet:> or <testnet:>).

Run:

```bash
python3 -m shoestring init --package mainnet config.ini
```

This command downloads the configuration files required to operate on the <mainnet:> network and creates the file
`config.ini`.

For <testnet:> nodes, use the `sai` package instead.
In later commands that use the `--network` parameter, specify `testnet` instead of `mainnet`.

```bash
python3 -m shoestring init --package sai config.ini
```

### 3. Configure Shoestring

The `config.ini` file generated in the previous step already contains most required settings.
Only a few values usually need to be customized for your node, and they are all located in the `#!ini [node]` section:

* The node [roles](../../textbook/nodes.md#roles), called _features_ by Shoestring.

    Set the desired node features from the list: `PEER`, `API`, `HARVESTER`, and `VOTER`.
    You can combine multiple features by separating them with a vertical bar `|`, for example:

    ```ini title="config.ini"
    [node]
    features = PEER | API
    ```

    Review the [roles](../../textbook/nodes.md#roles) page for detailed explanations.
    In summary:

    * `PEER`: The node participates in consensus.
    * `API`: The node exposes an API for client applications.
    * `HARVESTER`: The node participates in block creation and is eligible for <harvesting:> rewards.
        A minimum of 10'000 <XYM:> is required.
        See [Configure Harvesting](#configure-harvesting-optional) below.
    * `VOTER`: The node participates in <finalization:>.
        A minimum of 3'000'000 <XYM:> is required.
        See [Configure Voting](#configure-voting-optional) below.

* The `caCommonName` and `nodeCommonName` properties are used as Common Names (CN) in the
    [certificates](https://en.wikipedia.org/wiki/X.509) generated for the node.

    They do not affect node behavior, but they must not be empty or certificate generation will fail.

    For example:

    ```ini title="config.ini"
    [node]
    caCommonName = My Symbol Node CA
    nodeCommonName = My Symbol Node
    ```

* `API` nodes that should accept HTTPS requests must set the `apiHttps` property to `true`.
    Otherwise, the node will only accept HTTP requests.

    ```ini title="config.ini"
    [node]
    apiHttps = false
    ```

    Enabling HTTPS support also requires setting a valid `host` as shown next.

### 4. Configure the Node

An additional configuration file is required to define node-specific settings.
Without it, the node will not be properly usable.

Create a new file called `overrides.ini` with the following content:

```ini title="overrides.ini"
[node.localnode]
host = my-symbol-node.com
friendlyName = My Symbol Node
```

* `host` is the public IP address or hostname through which the rest of the network reaches your node.

    If you use an unreachable host (like `localhost` or `127.0.0.1`), your node may connect to the network,
    but other nodes will be unable to initiate connections to it.

* `friendlyName` is the human-readable name displayed in node lists such as <https://symbol.fyi/nodes>.

!!! info "Further Customization"

    The `overrides.ini` file can be used to modify any Catapult setting, for example,
    [network](../../devbook/reference/config/network-properties.md) or
    [node](../../devbook/reference/config/node-properties.md) properties.

    To do so, add a section using the syntax `[config-file-name.config-section]`,
    then define the desired properties below it.

    Note that setting any network property to a value different from those of the rest of the network will make the node
    <fork:>, effectively disconnecting it from the rest of the network.

`config.ini` configures Shoestring itself, while `overrides.ini` customizes the generated node software.

### 5. Create the Node Identity

Each node requires a cryptographic identity for secure communication with other nodes.
At the heart of this identity is a <private key:>, which corresponds to the node's <main key:|main account>.

This key is stored in a [PEM file](https://en.wikipedia.org/wiki/Privacy-Enhanced_Mail),
and there are two ways to obtain it:

* Create a new key with OpenSSL:

    ```bash
    openssl genpkey -algorithm ed25519 -out ca.key.pem
    ```

* Import an existing private key:

    ```bash
    python3 -m shoestring pemtool --output ca.key.pem
    ```

    This command asks you to paste your private key, but you can also provide it through a file with the
    `--input` parameter.
    Take special care not to leave private keys exposed in files or command history.

    !!! tip "Reuse the Bootstrap Main Key"

        If you want to reuse the same main account used by an existing Bootstrap node,
        the main private key can be obtained from the `target/addresses.yml` file in the node's Bootstrap installation.

        If the file is encrypted, run the following command first:

        ```sh
        symbol-bootstrap decrypt \
            --source target/addresses.yml \
            --destination target/addresses_plaintext.yml
        ```

        Then retrieve the key from the `target/addresses_plaintext.yml` file and
        **remove the plaintext file after extracting the key**.

        Once you have the private key, use the `pemtool` command as described above.

Both options create a PEM file named `ca.key.pem` that contains the private key.

!!! warning "This file must be kept secure"

Leave it in this directory for now.
It will be moved offline at a later stage.

!!! info "Main Account Address"

    To view the address and public key corresponding to this private key, you can use Shoestring's `pemview` tool:

    ```bash
    python3 -m shoestring pemview --input ca.key.pem --network mainnet
    ```

    The output is similar to this:

    ```text linenums="1"
        i     | loaded ca.key.pem
        i     |     address: TCUCHCSSTTXRU3BMOBMAQXY2WMGSBICEH6WB77I
        i     |  public key: 5DA31F82685B4F03924B096A1343F297CF4552F...
    ```

    The account's address is required to send funds to it.
    Funds are then needed to pay transaction fees, for example when enabling harvesting.

    You can also use the `--show-private` parameter to display the private key and
    [import it into the wallet](../wallet/import-account.md).
    This step allows you to manage the account more conveniently, but it is not required for this guide.

    Alternatively, you can use the `openssl` tool directly to show the account's public key:

    ```bash
    openssl pkey -in ca.key.pem -text
    ```

!!! tip "Import the remaining Bootstrap Keys"

    If you want to continue using the same keys from a previous node, like <VRF keys:>, <remote keys:>,
    or <voting keys:>, import them now with the following command:

    ```sh
    python3 -m shoestring import-bootstrap \
        --config config.ini \
        --bootstrap [Path to the Bootstrap target directory] \
        --include-node-key
    ```

    Note that the `--bootstrap` parameter typically points to the `target` directory inside the
    previous Bootstrap installation, for example `../bootstrap-node/target`.

    This command copies all necessary keys into the `bootstrap-import` directory,
    and updates `config.ini` to reference them.

    Note that no block data is imported: Upon first launch, the node synchronizes with the network by
    downloading the full blockchain.

    From this point onward, the previous Bootstrap installation is no longer needed.

### 6. Create the Node Installation

Once the configuration and key files are ready, create the node installation with:

```bash
python3 -m shoestring setup \
  --config ./config.ini \
  --overrides ./overrides.ini \
  --package mainnet \
  --directory . \
  --ca-key-path ./ca.key.pem
```

This command performs several tasks:

* Prepares the node directory structure.
* Generates certificates from the private key.
* Creates the Docker configuration files.
* Creates transactions that enable harvesting, if configured.

## Configure Harvesting (Optional)

If you enabled the `HARVESTER` role, one additional configuration step is required.
If not, this section can be skipped.

!!! tip "Reuse Harvesting Configuration from Bootstrap"

    If you imported all keys from a working Bootstrap installation as described above,
    and harvesting was already configured there, this section can also be skipped.

To enable harvesting, a <VRF key:> must be linked to your account through a special transaction.

For security reasons, the main account must also be linked through a separate transaction to a
<remote key:|remote account>.
This remote account is used by the node to sign newly created blocks, so it must remain on the node,
where it stays online and could be exposed if the server is ever compromised.

However, the remote account does not hold any funds, as all harvesting rewards are credited to
the linked main account, which can remain stored offline.

Shoestring's `setup` command has already created both transactions and stored them in a file named
`linking_transaction.dat`.
The following subsections explain how to sign the transactions in this file and announce them to the network.

### 7. Fund the Main Account

First, the node's main account needs to hold some <XYM:> to pay the fees required to announce these transactions.

Obtain the account's address as shown in [step 5 above](#5-create-the-node-identity) and transfer funds to it
from another account.
1 <XYM:> is typically enough.

### 8. Sign the Transactions

Shoestring's `signer` tool reads `linking_transaction.dat` and signs the transactions in it with the key stored in
`ca.key.pem` (the node's main account):

```bash
python3 -m shoestring signer \
    --config config.ini \
    --ca-key-path ca.key.pem \
    --save linking_transaction.dat
```

The signature is written back to `linking_transaction.dat` together with the transactions, so no new file is created.

The output looks similar to the following:

```text linenums="1"
    i     | Aggregate transaction: ((signature: 00000000...,
            signer_public_key: 5DA31F82..., version: 0x3,
            network: NetworkType.TESTNET,
            type: TransactionType.AGGREGATE_COMPLETE,
            fee: 0x0000000000015180,
            deadline: 0x0000001946C3C12A, )
            transactions_hash: CCA5A2CE..., transactions: [],
            cosignatures: [], )
    i     | Inner transactions:
    i     |   ((signer_public_key: 5DA31F82..., version: 0x1,
                network: NetworkType.TESTNET,
                type: TransactionType.ACCOUNT_KEY_LINK, )
                linked_public_key: E44E749E...,
                link_action: LinkAction.LINK, )
    i     |   ((signer_public_key: 5DA31F82..., version: 0x1,
                network: NetworkType.TESTNET,
                type: TransactionType.VRF_KEY_LINK, )
                linked_public_key: 88307270...,
                link_action: LinkAction.LINK, )
    i     | Signed transaction TransactionType.AGGREGATE_COMPLETE
            with hash: BE1DA35E...
```

Note that the two key-link transactions are wrapped inside a single <aggregate transaction:>, for convenience.

### 9. Announce the Transactions

Use the `announce-transaction` tool to submit the signed transactions to the network:

```bash
python3 -m shoestring announce-transaction \
    --config config.ini \
    --transaction linking_transaction.dat
```

The tool connects to an available node on the network and submits the signed transactions.
The output is similar to this:

```text linenums="1"
    i     | connecting to https://401-sai-dual.symboltest.net:3001
    i     | preparing to announce transaction 5E59A0A0... of type
            TransactionType.AGGREGATE_COMPLETE
    i     | transaction was successfully sent to the network
```

### 10. Wait for Confirmation

The `announce-transaction` command exits without waiting for confirmation, so the result must be verified manually.

From the output of the previous command, note the **URL** of the node where the transaction was submitted
(line 1) and the **hash** of the transaction (line 2).

Then open the URL in a browser, or query it with `curl`:
`{NODE_URL}/transactionStatus/{FULL_TRANSACTION_HASH}`.

For example, from the output above, the URL is:

[https://401-sai-dual.symboltest.net:3001/transactionStatus/5E59A0A0...](https://401-sai-dual.symboltest.net:3001/transactionStatus/5E59A0A009A885B8DA0A6E1CE18BAE26A83441414E595799F25DB94848CCDEEB)

!!! note "Check the same node where the transactions were submitted"

    If the node rejected the transactions without announcing them to the rest of the network,
    all other nodes will respond to this URL with a "transaction unknown" error,
    so always check the status of a transaction on the same node where you submitted it.

If the transactions are accepted, the node returns a message similar to this one:

```json
{
  "hash": "5E59A0A009A885B8DA0A6E1CE18BAE26A83441414E595799F25DB94848CCDEEB",
  "code": "Success",
  "deadline": "107120562182",
  "group": "confirmed"
}
```

The most common outcomes are:

| Result                       | Meaning                                                                                                                                  |
|------------------------------|------------------------------------------------------------------------------------------------------------------------------------------|
| **Confirmed**                | The transactions are confirmed. Continue with this guide.                                                                                |
| **Unconfirmed**              | The transactions are valid but not yet included in a block. Wait up to 30 seconds.                                                       |
| **Unknown**                  | The transactions were submitted to a different node, or too long ago, or not at all. Repeat from [Step 9](#9-announce-the-transactions). |
| **Insufficient funds**       | Fund the node's main account as explained in [step 7 above](#7-fund-the-main-account), then announce again.                              |
| **Signature Not Verifiable** | The transactions were probably not signed. Repeat from [Step 8](#8-sign-the-transactions).                                               |
| **Deadline expired**         | Transactions must be announced within 6 hours of running `setup`. See below.                                                             |

In case of expired deadlines, repeat [step 6](#6-create-the-node-installation) using the `--output-transaction-only`
parameter to create a fresh `linking_transaction.dat` and continue from [step 9](#9-announce-the-transactions) to
announce it.

Once the transactions are confirmed, the node is ready to participate in block creation using the <remote key:>
(created automatically during the setup process), and accrue any harvesting rewards on the <main key:|main account>
(obtained from the `ca.key.pem` file).

!!! warning "Importance Calculation Delay"

    A node has a higher chance of harvesting blocks, and therefore collecting rewards, when the <importance:> of
    its main account is higher.
    
    If you just funded the main account, importance takes 12 hours to update on <mainnet:> and 3 hours on <testnet:>.

## Configure Voting (Optional)

To create a <voting node:>, the following requirements must be met:

* Set the `VOTER` role when [configuring Shoestring](#3-configure-shoestring).
* The node's main account must hold 3'000'000 <XYM:>.

!!! info "Voting Keys Maintenance"

    During setup, Shoestring creates <voting keys:> for the node.
    For security reasons, these keys have a maximum lifespan of 180 days, so they must be renewed periodically.

    Once the node is up and running, read the [Maintaining a Node](./maintain.md#voting-key-renewal) guide to
    learn about the renewal process.

## Start the Node

The node is now configured and ready to be started, but one final security step remains:

!!! danger "Backup the Private Key"

    The `ca.key.pem` file contains the main account's private key, and it is NOT needed for normal node operation.
    This file should be moved offline after the node is properly configured.

    Copy `ca.key.pem` to secure storage, ideally offline, and **remove it from the machine that will run the node**.

Start the node containers with Docker:

```bash
docker compose up -d
```

The `-d` option runs the containers in detached mode, allowing the node to continue running after the terminal is closed.

!!! warning "Docker permissions"

    If you receive an error similar to:

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

Keep in mind that the node might take up to 5 minutes to shut down cleanly.
If interrupted, data might be left in an inconsistent state and require [recovery](#node-recovery).

## Verify the Node

Check that the node is running correctly:

```bash
python3 -m shoestring health --config config.ini --directory .
```

If the node started successfully, the command reports that the peer endpoint, REST API, and WebSocket services are
reachable (assuming these are the features you enabled):

```text
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
      i     | websocket connected to ws://127.0.0.1:3000/ws,
              subscribing and waiting for block
      i     | websocket received a block with height 226082
```

!!! info "Synchronization"

    Upon first launch, the node synchronizes with the network by downloading the full blockchain.

    This process can take from 24 to 48 hours.
    During this time, REST requests to the node, if it is running the API role,
    may be delayed and report an incorrect chain height.

## Node Recovery

In the event of an unclean shutdown, for example, due to power loss, lock files may remain in the `data`
folder that prevent the node from starting.

The following command attempts to recover the node and restore a valid state:

```bash
docker compose -f docker-compose-recovery.yaml up \
    --abort-on-container-exit
```

After the recovery command completes, check whether any `.lock` files still exist in the `data` directory.
If they have been removed, start the node again with `docker compose up -d`.

!!! warning "Last Resort"

    If the recovery command does not solve the issue, `reset-data` can be used.

    This command removes all downloaded blockchain data, so the node must synchronize again from the beginning.
    However, it preserves the node's configuration and keys.

    ```bash
    python3 -m shoestring reset-data \
        --config ./config.ini \
        --directory .
    ```

## Next Steps

Once the node is running, only occasional [maintenance tasks](./maintain.md) remain.
