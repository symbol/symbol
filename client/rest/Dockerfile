ARG UBUNTU_VERSION=22.04
ARG NODE_VERSION=18

FROM ubuntu:${UBUNTU_VERSION} as builder

ARG NODE_VERSION

RUN apt-get update && apt-get upgrade -y && apt-get install -y curl \
	&& curl -fsSL https://deb.nodesource.com/setup_${NODE_VERSION}.x | bash - \
	&& apt-get install -y nodejs

RUN apt-get install -y build-essential

WORKDIR /app
COPY . .
RUN npm uninstall . && rm -rf node_modules && npm install

FROM ubuntu:${UBUNTU_VERSION}

ARG NODE_VERSION

RUN apt-get update && apt-get upgrade -y && apt-get install -y curl \
	&& curl -fsSL https://deb.nodesource.com/setup_${NODE_VERSION}.x | bash - \
	&& apt-get install -y nodejs

WORKDIR /app
COPY --from=builder /app .

RUN node --version && npm --version
EXPOSE 3000
