FROM ubuntu:20.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# nodejs
RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash - \
	&& apt-get install -y nodejs

# rest dependencies
RUN apt-get install -y make gcc g++

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu

WORKDIR /home/ubuntu
