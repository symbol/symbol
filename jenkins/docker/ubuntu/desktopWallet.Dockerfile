FROM ubuntu:22.04

# install build dependencies for node-hid
RUN apt-get update && apt-get install -y \
	build-essential \
	python3 \
	libusb-1.0-0-dev \
	pkg-config \
	curl \
	gnupg

# nodejs
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
ARG NODEJS_VERSION=16
RUN mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_${NODEJS_VERSION}.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs
