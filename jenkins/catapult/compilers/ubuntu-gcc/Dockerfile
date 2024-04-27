ARG FROM_IMAGE=ubuntu:24.04
FROM ${FROM_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	gnupg2 \
	curl \
	xz-utils \
	wget \
	>/dev/null 
	
# Use the latest gcc version.
ARG COMPILER_VERSION=13
RUN apt-get update >/dev/null && \
	apt-get install -y \
	gcc-${COMPILER_VERSION} g++-${COMPILER_VERSION} >/dev/null && \
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 100 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100 && \
	update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/gcov-dump gcov-dump /usr/bin/gcov-dump-${COMPILER_VERSION} 100 && \
	update-alternatives --install /usr/bin/gcov-tool gcov-tool /usr/bin/gcov-tool-${COMPILER_VERSION} 100 && \
	rm -rf /var/lib/apt/lists/*
