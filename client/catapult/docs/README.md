# Developer Notes

This document is intended for readers interested in catapult-client development and or willing to compile catapult-client from source.

## Hardware requirements

The scripts shared in these documents have been tested on servers with the following hardware:

* CPU: 4+ cores, with **little endian** architecture
* Memory: 8+ GB
* HD: 100+ GB

Runtime server requirements are network dependent.
For example, networks with higher throughput will likely have higher requirements.

Although you might be able to run the software in less powerful instances, you might encounter some issues while installing or running the node.

## Port requirements

The port ``7900`` is required by catapult-client to communicate between nodes.
Make sure that the node's host is accessible from the internet and that the port is open and available.

## Building catapult-client

We have tested the build process on Linux, Apple (macOS), and Windows. The instructions for each platform are available in separate documents.

Pick the guide for your platform. Each platform guide covers two dependency-management options: [vcpkg](https://github.com/microsoft/vcpkg) (recommended) and [Conan](https://conan.io) (used by CI).

* [Build on Linux](BUILD-linux.md)
* [Build on Apple (macOS)](BUILD-apple.md)
* [Build on Windows](BUILD-windows.md)

Alternative flows:

* [Build with Docker](BUILD-docker.md)
* [Build manually](BUILD-manual.md) (from-source dependencies, Linux/macOS)

### Prerequisites

Required (all platforms)

* [CMake](https://cmake.org/download/) >= 3.25
* [Git](https://git-scm.com/) >= 2.25
* Python 3.x
* A dependency manager: [vcpkg](https://github.com/microsoft/vcpkg) (recommended) or [Conan](https://conan.io) >= 2.0

Per platform

* Linux / macOS: [Ninja](https://ninja-build.org/) and `pkg-config`
* Windows: Visual Studio 2022 (>= 17.5) or 2026, or the matching Build Tools, with the "Desktop development with C++" and "C++ CMake tools" components

### Compilers

Catapult requires a C++17 compiler. The supported minimums are:

Linux:

* GCC >= 8 (GCC >= 11 strongly recommended)
* Clang >= 14

macOS:

* Apple Clang >= 14 (Xcode 14 or later)

Windows:

* MSVC (Visual Studio 2022 >= 17.5, or 2026)


### Sanitizers

There are a few false positives when running sanitizers on targets
compiled with clang 9.
When building sanitizers, `sanitizer_blacklist.txt` file is used.

When running thread sanitizer, there are following suppressions required:

* for false positive in libc++ `shared_ptr`:
* for false positive in boost's 1.74 executor (ref-counted)
* for false positive in boost logger, in server logger is always initialized from a single thread

```suppresions
race:~weak_ptr
race:~executor
race:global_logger::get()
```

## Running a private network

* [How to create a new network](RUNNETWORKLIN.md)

* [Network configuration](https://docs.symbol.dev/guides/network/configuring-network-properties.html)

## Running a peer node

* [How to create a node and connect to an existing network](RUNPEERLIN.md)

* [Node configuration](https://docs.symbol.dev/guides/network/configuring-node-properties.html)
