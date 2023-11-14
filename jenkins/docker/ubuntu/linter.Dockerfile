FROM ubuntu:22.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# install npm-groovy-lint
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
ARG NODE_MAJOR=18
RUN apt-get install -y default-jre ca-certificates curl gnupg \
	&& mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs \
	&& npm install -g npm-groovy-lint@11.1.1

# install python
RUN apt-get install -y python3-pip

# install shellcheck and gitlint
RUN apt-get install -y shellcheck \
	&& pip install gitlint

# install ripgrep and yamllint
RUN apt-get install -y ripgrep \
	&& pip install yamllint

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu
ENV PATH=$PATH:/home/ubuntu/.local/bin

WORKDIR /home/ubuntu
