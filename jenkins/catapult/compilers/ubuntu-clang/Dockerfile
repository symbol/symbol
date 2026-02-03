ARG FROM_IMAGE=ubuntu:25.10
FROM ${FROM_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	curl \
	xz-utils \
	wget 

# Use the latest Clang version from Ubuntu.
ARG COMPILER_VERSION=21
RUN apt-get update >/dev/null && \
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
	clang --version

# set default user (used by jenkins)
ARG HOME_DIR=/home/ubuntu
USER ubuntu
WORKDIR ${HOME_DIR}
