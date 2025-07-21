FROM ubuntu:22.04

# install build dependencies for node-hid
RUN apt-get update && apt-get install -y \
	build-essential \
	curl \
	git \
    gnupg \
    libusb-1.0-0-dev \
	pkg-config \
    python3

# nodejs
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
ARG NODEJS_VERSION=16
RUN mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_${NODEJS_VERSION}.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu
