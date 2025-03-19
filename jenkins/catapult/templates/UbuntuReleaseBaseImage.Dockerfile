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
ARG HOME_DIR=/home/catapult
USER ubuntu
WORKDIR ${HOME_DIR}
