# image name required as ARG
ARG FROM_IMAGE='ubuntu:24.04'
ARG DEBIAN_FRONTEND=noninteractive

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"
RUN apt-get -y update && apt-get install -y \
	gdb \
	openssl \
	&& \
	rm -rf /var/lib/apt/lists/*

# add ubuntu user (used by jenkins)
# create the user for debian image
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
ARG HOME_DIR=/home/ubuntu
USER ubuntu
WORKDIR ${HOME_DIR}
