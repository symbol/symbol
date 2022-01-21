FROM ubuntu:20.04

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	gnupg2 \
	curl \
	xz-utils \
	wget \
	>/dev/null && \
	apt-get update >/dev/null && \
	apt-get install -y \
	gcc-10 g++-10 \
	>/dev/null && \
	update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 100 && \
	update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 100 && \
	update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 100 && \
	update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100 && \
	rm -rf /var/lib/apt/lists/*
