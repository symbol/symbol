# Maintaining a Node

## Monitoring

Regular monitoring helps detect problems before they affect harvesting, voting, or API availability.
A healthy node should remain online, synchronized with the network, and responsive to requests.

Checking the following items periodically can prevent most unexpected outages.
Terminal commands assume they are run from the node's directory.

* **Check the REST Health Endpoint** remotely

    If the REST Gateway is enabled, the node also exposes the health endpoint <get:/node/health>.

    A healthy response looks similar to the following:

    ```json
    {
        "status": {
            "apiNode": "up",
            "db": "up"
        }
    }
    ```

    If any component reports `down`, further investigation is required.

* **Check Node Health with Shoestring**
{: #check-node-health-with-shoestring :}

    Shoestring includes a built-in health check command:

    ```bash
    python3 -m shoestring health --config config.ini --directory .
    ```

    This command performs a basic status check of the local node and reports whether its services appear to be
    operating normally, and all the required keys are correctly registered and not
    [nearing expiration](#key-and-certificate-renewals).

    A healthy node report looks like this:

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

* **Check Docker Container Status**

    Shoestring nodes run inside Docker containers.
    Use the following command to verify that all required containers are running:

    ```bash
    docker compose ps
    ```

    Containers stuck in a restart loop or shown as exited usually indicate a configuration or startup problem.

* **View Logs**
{: #view-logs :}

    Logs are often the fastest way to diagnose node issues.

    To view recent logs:

    ```bash
    docker compose logs --tail=50
    ```

    To follow logs in real time (stop with `Ctrl+C`):

    ```bash
    docker compose logs -f
    ```

    You can optionally specify the service you want to examine, such as `client`, `db`, or `broker`.

    Look for repeated errors, database problems, connection failures, or warnings about missing files.

* **Verify Network Synchronization**

    A running node is not necessarily a synchronized node.
    Compare the current block height of your node with public explorers or node lists.

    Useful resources include:

    * [Symbol Explorer](https://symbol.fyi/nodes)
    * [Nodewatch](https://nodewatch.symbol.tools/symbol/nodes)
    * [Symbol Node List](https://symbol-tools.com/symbolTools/view/tool/nodeList.html)

    If your node remains significantly behind other public nodes for an extended period, further investigation may
    be required.

## Disk Space Management

Insufficient disk space is a common cause of node instability.
A node that runs out of available storage stops synchronizing.

Keep track of the available disk space, either locally, remotely, or using your infrastructure provider's monitoring
tools if you use a cloud server.

It is better to stop the node cleanly before storage becomes critically low.

If you need to move to a bigger storage, the node's directory can be safely moved,
**as long as the node is fully stopped** before moving the files.

!!! tip
    To minimize issues, always follow the recommended hardware and storage guidelines in the
    [Installation](./install.md#hardware) guide.

## Restarting and Recovery

Unexpected shutdowns, power loss, or system errors may occasionally stop the node or prevent it from starting
correctly.
Most issues can be resolved with a clean restart and a few basic checks.

* **Stop the Node Cleanly**
{: #stop-the-node-cleanly :}

    Before performing maintenance or recovery tasks, stop all services gracefully:

    ```bash
    docker compose down
    ```

    This allows containers to shut down cleanly and reduces the risk of database corruption.
    Keep in mind that the node might take up to 5 minutes to shut down cleanly.

* **Start the Node Again**
{: #start-the-node-again :}

    To start the node after shutdown:

    ```bash
    docker compose up -d
    ```

    After startup, allow a few moments for all services to initialize.

* **Check Container Status**

    Verify that all required containers are running:

    ```bash
    docker compose ps -a
    ```

    If any container exits immediately or repeatedly restarts, [inspect the logs](#view-logs).

* **Stale Lock Files**

    If the node was interrupted unexpectedly, `.lock` files may remain in the `data` directory and prevent startup.

    If you find any such file, read the [Node Recovery](./quickstart.md#node-recovery) section.

## Updating Node Software

Keeping the operating system and node software updated improves stability, compatibility, and security.

* **Update Shoestring**

    Shoestring itself is installed as a Python package.
    To update it to the latest available version:

    ```bash
    python3 -m pip install --upgrade symbol-shoestring
    ```

    After the update, confirm the installed version:

    ```bash
    python3 -m pip show symbol-shoestring
    ```

* **Update Node Containers**

    Shoestring provides the `upgrade` command to download and install the latest version of all Symbol components.
    Make sure you run it regularly.

    !!! warning

        [Stop your node](#stop-the-node-cleanly) before upgrading it, and then [restart it](#start-the-node-again).

    ```bash
    python3 -m shoestring upgrade \
        --config config.ini \
        --directory . \
        --overrides overrides.ini
    ```

    After restarting the node, [confirm that it is operating normally](#monitoring).

    !!! note "Configuration Changes"

        In rare cases, an update may change the format of the Shoestring configuration file, making it incompatible
        with the existing `config.ini`.

        Such breaking updates would be widely announced and may require backing up the current `config.ini`,
        [downloading a fresh version with the `init` command](./quickstart.md#2-generate-the-network-configuration),
        and manually merging the previous changes into it.

## Key and Certificate Renewals

Several keys and certificates are required for normal node operation.

For security reasons, none of them are intended to remain valid indefinitely, so they must be renewed periodically.

Shoestring provides renewal commands to simplify this process, as shown in the following sections.

### TLS Certificate Renewal

Communication with other peer nodes is encrypted using a [TLS](https://en.wikipedia.org/wiki/Transport_Layer_Security)
certificate that Shoestring generated during the [setup phase](./quickstart.md#6-create-the-node-installation),
using the node's <main key:>.

This certificate has a default validity period of 375 days and can be renewed with the `renew-certificates` command:

```bash
python3 -m shoestring renew-certificates \
    --config config.ini \
    --directory . \
    --ca-key-path ca.key.pem
```

If a `File Not Found` error is shown, try using an absolute path for the `--directory` parameter.

The [`health` command](#check-node-health-with-shoestring) warns when expiration is approaching and the certificate
should be renewed.
You can also check the current certificate's expiration date remotely on the
[Symbol Node List](https://symbol-tools.com/symbolTools/view/tool/nodeList.html).

!!! danger "Protect the Main Account Key"

    Creating new certificates requires access to the PEM file containing the node's
    <main key:|main account> private key.

    This file should be stored securely and should not normally remain on the node itself.

!!! warning "Retaining the Node Key"

    By default, the command above generates a new node key pair.

    This changes the public key that <delegated harvesting:|delegated harvesters> use when registering to the node.

    Existing delegators normally continue to work without interruption, **unless the node must perform a full
    resynchronization** of the blockchain.

    To continue using the existing node key and avoid this possibility, use the `--retain-node-key` parameter when
    renewing the certificate.

### Voting Key Renewal

<Voting nodes:|Voting nodes> require <voting keys:>, which must be registered for a limited validity period.

For security reasons, each voting key is valid for a maximum of 180 days (roughly six months), so keys must be renewed
periodically to keep the node eligible for voting on finalization, both on <mainnet:> and <testnet:>.

Only one key is required for the voting process at any given time, but up to three keys can be registered
simultaneously.
This allows overlapping validity periods, so a backup key remains available when another key expires.

When Shoestring [creates a voting node](./quickstart.md#configure-voting-optional), it already registers a voting key.
It also provides a command to renew it:

```bash
python3 -m shoestring renew-voting-keys --config config.ini --directory .
```

This command creates new keys that become active once the current one expires,
and prepares the necessary transactions to:

* Unlink expired voting keys.
* Link the new keys to the node.

The transactions are stored in a file named `renew_voting_keys_transaction.dat`.

They must then be signed and announced manually, following the process described in the
[Creating and Running a Node](./quickstart.md#8-sign-the-transactions) guide.

!!! danger "Protect the Main Account Key"

    Signing these transactions requires access to the PEM file containing the node's
    <main key:|main account> private key.

    This file should be stored securely and should not normally remain on the node itself.

## Backups

Unlike many other services, a Symbol node does not usually require backups of its blockchain database.

The blockchain is already replicated across many independent nodes.
If local chain data is lost or corrupted, the node can normally be resynchronized from the network.

For this reason, backups should focus on local files that cannot be recovered automatically:

* Shoestring configuration file `config.ini`
* Node identity key file `ca.key.pem`
* Node customization `overrides.ini`

If harvesting is enabled, a full resynchronization normally regenerates the `harvesters.dat` file that tracks the
currently registered delegators.

For that reason, backing up this file is usually unnecessary unless the blockchain is restored
[from a snapshot](https://github.com/symbol/product/tree/dev/tools/shoestring#need-to-resync-your-node).
