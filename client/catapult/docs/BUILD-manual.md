# Building manually

This guide should mostly work for Linux and OS X operating systems.
It does not include instructions for Windows.
For Windows, [Build with CONAN](BUILD-conan.md) is strongly encouraged.

## Prerequisites

- ``git``.
- About 15 GB of free disk space.

These instructions have been verified to work on Ubuntu 20.04 with 8 GB of RAM and 4 CPU cores.

## Step 1: Clone catapult-server

As usual, clone the git repository:

```sh
git clone https://github.com/nemtech/catapult-server.git
cd catapult-server
```

## Step 2: Install build prerequisites

The snippet below installs the required system-wide prerequisites on apt-based systems (like Ubuntu or Debian), creates a directory in ``$HOME/cat_deps_dir`` containing all build dependencies and points the environment variable `CAT_DEPS_DIR` to it.

```sh
sudo scripts/configure-manual.sh install system_reqs
export CAT_DEPS_DIR=$HOME/cat_deps_dir
```

> **NOTE**:
> If you want to install the dependencies to a place other than ``$HOME/cat_deps_dir`` you can define the ``CAT_DEPS_DIR`` environment variable **before** executing the script.
>
> If you want the `CAT_DEPS_DIR` environment variable to persist across sessions make sure to include the last line in the `~/.profile` or `~/.bashrc` files.

## Optional step: Download, build and install all dependencies from source

Type this into a terminal:

```sh
scripts/configure-manual.sh install deps
```

> **NOTE**:
> If you only want to download the dependencies (without building and installing them) enter the command ``scripts/configure-manual.sh download deps`` instead. Type ``scripts/configure-manual.sh --help`` for the summary of all commands.

## Step 3: Prepare build directory

To create or update the ``_build`` directory type into a terminal:

```sh
scripts/configure-manual.sh
```

This will handle the previous optional step if you skipped it.

## Step 4: build catapult

Finally, to start compiling type into a terminal:

```sh
cd _build
ninja -j8
```

## Step 5: Installation

Once the build finishes successfully, the tools in ``_build/bin`` are ready to use. Optionally, they can be made available globally by running:

```sh
sudo ninja install
```

> **NOTE:**
> You can change the default installation location by running Step 3 with the ``CMAKE_INSTALL_PREFIX`` environment variable set, e.g. ``CMAKE_INSTALL_PREFIX=... scripts/configure-manual.sh``. In this case you might not require ``sudo`` when invoking ``ninja install``.

Regardless of whether the tools are installed globally or not, their dependencies must be accessible before running them so make sure to update ``LD_LIBRARY_PATH``:

```sh
export LD_LIBRARY_PATH=$CAT_DEPS_DIR/boost/lib:$CAT_DEPS_DIR/facebook/lib:$CAT_DEPS_DIR/google/lib:$CAT_DEPS_DIR/mongodb/lib:$CAT_DEPS_DIR/zeromq/lib
```

Make sure this environment variable persists across sessions by including the above line in the `~/.profile` or `~/.bashrc` files.

## Step 6: Verification

Verify that the tools are working correctly by running:

```sh
bin/catapult.tools.address --help
```
