# image name required as ARG
ARG FROM_IMAGE=''
ARG DEBIAN_FRONTEND=noninteractive

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"
RUN apt-get -y update && apt-get install -y \
	gdb \
	openssl \
	&& \
	rm -rf /var/lib/apt/lists/*
