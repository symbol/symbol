ARG FROM_IMAGE=ubuntu:26.04
FROM ${FROM_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	curl \
	gnupg \
	lsb-release \
	software-properties-common \
	xz-utils \
	wget

# Use the latest Clang version from Ubuntu.
ARG COMPILER_VERSION=23
RUN apt-get update >/dev/null && \
	wget https://apt.llvm.org/llvm.sh && \
	chmod +x llvm.sh && \
	./llvm.sh ${COMPILER_VERSION} && \
	apt-get install -y \
	llvm-${COMPILER_VERSION} \
	llvm-${COMPILER_VERSION}-tools \
	llvm-${COMPILER_VERSION}-dev \
	liblldb-${COMPILER_VERSION}-dev \
	clang-tools-${COMPILER_VERSION} \
	libc++-${COMPILER_VERSION}-dev \
	libc++abi-${COMPILER_VERSION}-dev \
	libunwind-${COMPILER_VERSION}-dev && \
	rm llvm.sh && \
	apt-get clean && \
	update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100 && \
	clang --version

# Set environment variables so build systems (like CMake) pick it up automatically
ENV CC=clang
ENV CXX=clang++

# set default user (used by jenkins)
ARG HOME_DIR=/home/ubuntu
USER ubuntu
WORKDIR ${HOME_DIR}
