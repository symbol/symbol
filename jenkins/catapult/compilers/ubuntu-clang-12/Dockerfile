FROM ubuntu:20.04

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	curl \
	xz-utils \
	wget \
	>/dev/null && \
	echo 'deb http://apt.llvm.org/focal/ llvm-toolchain-focal-12 main' > /etc/apt/sources.list.d/clang.list && \
	wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
	apt-get update >/dev/null && \
	apt-get install -y \
	clang-12 clang-tools-12 libc++-12-dev libc++abi-12-dev \
	>/dev/null && \
	update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 100 && \
	update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-12 100 && \
	update-alternatives --install /usr/bin/cc cc /usr/bin/clang 100 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++ 100 && \
	rm -rf /var/lib/apt/lists/*
