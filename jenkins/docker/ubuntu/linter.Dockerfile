FROM ubuntu:24.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# install npm-groovy-lint which requires java 17
ENV NODE_OPTIONS="--dns-result-order=ipv4first"
ARG NODE_MAJOR=18
RUN apt-get install -y openjdk-17-jre ca-certificates curl gnupg \
	&& mkdir -p /etc/apt/keyrings \
	&& curl -fsSL https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key | gpg --dearmor -o /etc/apt/keyrings/nodesource.gpg \
	&& echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_$NODE_MAJOR.x nodistro main" \
	| tee /etc/apt/sources.list.d/nodesource.list \
	&& apt-get update \
	&& apt-get install -y nodejs \
	&& npm install -g npm-groovy-lint

# install python
RUN apt-get install -y python3-pip python3-venv

# install shellcheck and ripgrep
RUN apt-get install -y shellcheck ripgrep


# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu
ENV PATH=$PATH:/home/ubuntu/.local/bin

# create a virtual environment
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install common python packages
RUN python3 -m pip install --upgrade gitlint isort lark pycodestyle pylint PyYAML yamllint
