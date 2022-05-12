FROM ubuntu:20.04

# install dependencies (install tzdata first to prevent 'geographic area' prompt)
RUN apt-get update \
	&& apt-get install -y tzdata \
	&& apt-get install -y openjdk-11-jdk-headless git curl libssl-dev maven ca-certificates \
	&& update-ca-certificates

# install python
RUN apt-get install -y python3-pip

# install shellcheck and gitlint
RUN apt-get install -y shellcheck \
	&& pip install gitlint

# codecov uploader
RUN curl -Os https://uploader.codecov.io/v0.1.20/linux/codecov \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu

WORKDIR /home/ubuntu
