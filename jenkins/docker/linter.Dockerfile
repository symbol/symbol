FROM ubuntu:20.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# install npm-groovy-lint
RUN apt-get install -y default-jre \
	&& curl -fsSL https://deb.nodesource.com/setup_16.x | bash - \
	&& apt-get install -y nodejs \
	&& npm install -g npm-groovy-lint

# install shellcheck and python
RUN apt-get install -y shellcheck python3-pip

# install ripgrep
RUN apt-get install -y ripgrep

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu

WORKDIR /home/ubuntu
