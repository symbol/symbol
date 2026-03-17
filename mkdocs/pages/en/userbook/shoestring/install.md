# Installing Shoestring

This page explains how to install the Shoestring tool and verify that it is working correctly.

Shoestring is distributed as a Python package and can be installed using `pip`.

## Requirements

### Software

Before installing Shoestring, ensure the following software is available on the system:

* Python 3.10 or newer.
* `pip` (Python package manager).
* [Docker](https://www.docker.com).
* [OpenSSL](https://www.openssl.org/) command-line tools.

    ??? info "Installing OpenSSL"

        The **OpenSSL command-line tools** are required to generate the cryptographic keys used by the node.
        On many systems they are already installed or available through the operating system's package manager.
        If they are not present, they can be downloaded from <https://openssl-library.org/source/>.
        
        To check whether OpenSSL is available on the system, run `openssl version`.
        If the command prints the installed version, the tools are correctly installed and available in the system path.

### Hardware

Running a blockchain node is **very demanding** in terms of disk space, memory and CPU resources.
Failure to meet the following minimum requirements will produce a node that will struggle to keep up with the rest of
the network.
The global blockchain will not be affected but the node will rarely be eligible to collect any node rewards.

It is **strongly recommended** to use **dedicated CPU and RAM**.
When they are shared (as is the case on some Virtual Server providers) performance is heavily impacted.

=== "Minimum Requirements"

    | Requirement    | Peer node     | API node      | Dual / Voting node |
    |----------------|---------------|---------------|--------------------|
    | **CPU**        | 2 cores       | 4 cores       | 4 cores            |
    | **RAM**        | 8 GB          | 16 GB         | 16 GB              |
    | **Disk size**  | 500 GB        | 750 GB        | 750 GB             |
    | **Disk speed** | 1500 IOPS SSD | 1500 IOPS SSD | 1500 IOPS SSD      |

=== "Recommended Requirements"

    | Requirement    | Peer node     | API node      | Dual / Voting node |
    |----------------|---------------|---------------|--------------------|
    | **CPU**        | 4 cores       | 8 cores       | 8 cores            |
    | **RAM**        | 16 GB         | 32 GB         | 32 GB              |
    | **Disk size**  | 500 GB        | 750 GB        | 750 GB             |
    | **Disk speed** | 1500 IOPS SSD | 1500 IOPS SSD | 1500 IOPS SSD      |

## Installing with pip

Install Shoestring using the Python package manager:

```bash
pip install symbol-shoestring
```

This command installs the Shoestring tool and its dependencies.

!!! warning "Troubleshooting"

    On some systems, installing Shoestring may require additional system packages,
    because some Python dependencies are built from source.

    If installation fails with errors related to missing headers, libraries, or compiler tools,
    install the required **development packages** for your system and run the installation again.

    Common symptoms include errors mentioning `pyconfig.h`, OpenSSL headers, or missing build toolchains.

    On Ubuntu and Debian, it is typically enough to install:

    ```bash
    sudo apt install python3-dev libssl-dev
    ```

    Then run the Shoestring installation again.

## Verifying the Installation

After installation, verify that the tool is available by running:

```bash
python3 -m shoestring --help
```

If the installation succeeded, the command prints a list of available Shoestring commands

To see the options available for a specific command, use:

```bash
python3 -m shoestring <command> --help
```

## Updating Shoestring

To upgrade Shoestring to the latest version, run:

```bash
pip install --upgrade symbol-shoestring
```

## Next Steps

After installing Shoestring, the next step is to [create and start a node](./quickstart.md).
