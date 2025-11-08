ARG UBUNTU_VERSION=24.04
ARG NODE_MAJOR=20

FROM ubuntu:${UBUNTU_VERSION} as builder

ARG NODE_MAJOR
ARG NODE_OPTIONS="--dns-result-order=ipv4first"

RUN apt-get update && apt-get upgrade -y && apt-get install -y curl gnupg \
	&& mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs

RUN apt-get install -y build-essential

WORKDIR /app
COPY . .
RUN npm uninstall . && rm -rf node_modules && npm install --omit=dev

FROM ubuntu:${UBUNTU_VERSION}

ARG NODE_MAJOR

RUN apt-get update && apt-get upgrade -y && apt-get install -y curl gnupg \
	&& mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs

USER ubuntu
WORKDIR /app
COPY --chown=1000:1000 --from=builder /app .

RUN node --version && npm --version
EXPOSE 3000
