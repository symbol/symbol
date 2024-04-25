# Building with Docker

For convenience and reproducibility, Docker images are available which already contain the required tools to build the Catapult Server, with the appropriate versions.

This guide explains how to use these Docker images and gives a bit of insight into how these images are built.

All scripts used in this guide can be found in the ``jenkins/catapult`` folder after cloning the repository.

## Prerequisites

- git.
- Docker.
- Python 3.
- About 15 GB of free disk space.

These instructions have been verified to work on Ubuntu 24.04 with 8 GB of RAM and 4 CPU cores. **The scripts used are not ready for Windows yet**.

## Step 1: Clone symbol repo

```sh
git clone https://github.com/symbol/symbol.git
```

## Step 2: Compiling the client

The script ``jenkins/catapult/runDockerBuild.py`` prepares the directory structure and fires compilation within the appropriate docker image.

Launch it from the root folder:

```bash
python3 jenkins/catapult/runDockerBuild.py \
    --compiler-configuration jenkins/catapult/configurations/gcc-latest.yaml \
    --build-configuration jenkins/catapult/configurations/release-private.yaml \
    --operating-system ubuntu \
    --user "$(id -u):$(id -g)" \
    --source-path client/catapult
    --destination-image-label gcc-latest-main-9273d6c5
```

Note the used ``--compiler-configuration`` and ``--operating-system`` parameters that build tools for **Ubuntu + gcc10**.

Note also how the ``--user`` parameters match the local user and group.

Upon successful completion the ``output/binaries/bin`` folder contains the produced binaries. However, the dependencies in ``output/binaries/deps`` must be accessible so make sure to add this folder to the ``LD_LIBRARY_PATH`` environment variable (Linux) or ``DYLD_LIBRARY_PATH`` (Mac).

  One way of doing this is by running this from the ``output`` directory:

  ```sh
  export LD_LIBRARY_PATH=$PWD/binaries/deps
  ```

  You will need to run this line every new session, unless you add it at the end of your ``~/.bashrc`` or ``~/.profile`` files.

### Build process details

The `runDockerBuild.py` script creates a bunch of directories and creates lengthy docker command line, which looks like this:

```bash
docker run \
      --rm \
      --user=1000:1000 \
      --env=CC=gcc \
      --env=CCACHE_DIR=/ccache \
      --env=CXX=g++ \
      --volume=/FULL_PATH/catapult:/catapult-src \
      --volume=/FULL_PATH/output/binaries:/binaries \
      --volume=/jenkins_cache/ccache/release:/ccache \
      --volume=/jenkins_cache/conan/gcc:/conan \
      symbolplatform/symbol-server-build-base:ubuntu-gcc-12-skylake \
      python3 \
      /catapult-src/scripts/build/runDockerBuildInnerBuild.py \
      --compiler-configuration=/catapult-src/scripts/build/configurations/gcc-latest.yaml \
      --build-configuration=/catapult-src/scripts/build/configurations/release-private.yaml
```

Unfortunately, right now `/jenkins_cache` is hard-coded. This will be modified in the future.

The script that fires actual compilation is `runDockerBuildInnerBuild.py`. To understand the exact steps you might want to read [the manual build instructions](BUILD-manual.md) Inside the Docker container, this script:

1. Sets up directories and ``ccache``.
2. Fires up ``cmake``.
3. Fires up the actual compilation using ``ninja publish`` followed by ``ninja``.
4. Makes initial installation using ``ninja install`` (which installs to volume mounted in ``/binaries``).
5. Copies the dependencies into directories inside ``/binaries``.

Note that step 4 and 5 copy the files into directory ``$PWD/output/binaries``.

### Release image preparation

The next thing ``runDockerBuild.py`` does is firing up another container that prepares the final release image. The goal is to have a clean slate with only the compilation artifacts present, but none of the intermediate compilation files.

```bash
docker run \
      --cidfile=gcc-latest-main-9273d6c5.cid \
      --volume=/FULL_PATH/jenkins/catapult:/scripts \
      --volume=/FULL_PATH/output:/data \
      symbolplatform/symbol-server-test-base:ubuntu \
      python3 \
      /scripts/runDockerBuildInnerPrepare.py \
      --disposition=private
```

This copies the compiled artifacts into ``/usr/catapult`` directory inside the image and commits it.

## How the base images are created

> **NOTE**:
> This section is informative. The previous one explained how to build the Catapult Server using ready-made Docker images containing the necessary tooling. This section summarizes the process used to build these images.

Base images contain all tools needed to build the Catapult Server for a variety of Operating Systems and compilers.

They are created running ``baseImageDockerfileGenerator.py`` multiple times, with each run adding a different "layer" to the image. Each layer contains a different set of tools.

### Ready-made base images

Compiler images' ``Dockerfiles`` are available in the [scripts/build/compilers directory](https://github.com/symbol/symbol/tree/dev/jenkins/catapult/compilers).

These images are built automatically via Docker Hub and are available in the [Docker Hub symbol-server-compiler repository](https://hub.docker.com/repository/docker/symbolplatform/symbol-server-compiler).

The process described next allows building several combinations of Operating Systems and compilers. The following is the list of combinations automatically built by Docker Hub:

- Ubuntu (24.04)
  - gcc-12
  - gcc-13
  - clang-17
  - clang-18
  - clang + sanitizers (address + undefined behavior)
  - clang + sanitizers (threads)
- Fedora (40)
  - gcc-14
- Debian (12)
  - gcc-10 (this is the newest gcc avail on Debian)

### Building your own base image

This is a sample invocation that creates an image with all layers. Note the used ``--compiler-configuration`` and ``--operating-system`` parameters that configure this image as **Ubuntu + gcc10**.

Note also that the architecture is defined in ``configurations/gcc-latest.yaml`` as ``skylake``.

1. Create the first intermediate image with ``cmake`` and various system packages:

   ```bash
   python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
       --compiler-configuration jenkins/catapult/configurations/gcc-latest.yaml \
       --operating-system ubuntu \
       --versions ./jenkins/catapult/versions.properties \
       --layer os
   ```

2. Creates the second intermediate image with boost build:

   ```bash
   python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
       --compiler-configuration jenkins/catapult/configurations/gcc-latest.yaml \
       --operating-system ubuntu \
       --versions ./jenkins/catapult/versions.properties \
       --layer boost
   ```

3. Create the third intermediate image with all other dependencies (``mongo`` + ``mongo-cxx``, ``libzmq`` + ``cppzmq``, ``rocksdb``):

   ```bash
   python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
       --compiler-configuration jenkins/catapult/configurations/gcc-latest.yaml \
       --operating-system ubuntu \
       --versions ./jenkins/catapult/versions.properties \
       --layer deps
   ```

4. Create the final layer, that adds:
   - System ``libssl-dev`` package.
   - Things needed to run tests and lints: ``googletest``, ``google benchmark``, ``pip``, ``pycodestyle``, ``pylint``, ``pyyaml``.

   ```bash
   python3 ./jenkins/catapult/baseImageDockerfileGenerator.py \
       --compiler-configuration jenkins/catapult/configurations/gcc-latest.yaml \
       --operating-system ubuntu \
       --versions ./jenkins/catapult/versions.properties \
       --layer test
   ```

The final image name is `symbolplatform/symbol-server-build-base:ubuntu-gcc-latest-skylake` (based on the used configuration).
