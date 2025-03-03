ARG FROM_IMAGE=debian:12
FROM ${FROM_IMAGE}

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
	gcc g++ \
	>/dev/null && \
	rm -rf /var/lib/apt/lists/*

RUN id -u "debian" || useradd --uid 1000 -ms /bin/bash debian
USER debian
WORKDIR /home/debian
ENV PATH=$PATH:/home/debian/.local/bin
