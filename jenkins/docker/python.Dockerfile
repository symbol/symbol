FROM ubuntu:20.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git

# install shellcheck and python
RUN apt-get install -y shellcheck python3-pip

# sdk dependencies
RUN apt-get install -y zbar-tools

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu

WORKDIR /home/ubuntu
