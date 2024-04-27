ARG FROM_IMAGE=ubuntu:24.04
FROM ${FROM_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	curl \
	xz-utils \
	wget 

# Use the latest Clang version.
ARG COMPILER_VERSION=18
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor > /etc/apt/trusted.gpg.d/clang.gpg  && \
	echo "deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-${COMPILER_VERSION} main" > /etc/apt/sources.list.d/clang.list && \
	apt-get update >/dev/null && \
	apt-get install -y \
	clang-${COMPILER_VERSION} \
	clang-tools-${COMPILER_VERSION} \
	libc++-${COMPILER_VERSION}-dev \
	libc++abi-${COMPILER_VERSION}-dev \
	libunwind-${COMPILER_VERSION}-dev && \
	update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100 && \
	rm -rf /var/lib/apt/lists/* && \
	clang --version
