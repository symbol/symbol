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
