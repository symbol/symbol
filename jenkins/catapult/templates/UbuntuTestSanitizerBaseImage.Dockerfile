# image name required as ARG
ARG FROM_IMAGE=''
ARG DEBIAN_FRONTEND=noninteractive

FROM ${FROM_IMAGE}
LABEL maintainer="Catapult Development Team"
RUN apt-get -y update && apt-get install -y \
	bison \
	gdb \
	git \
	flex \
	python3 \
	python3-ply \
	python3-pip \
	shellcheck \
	inotify-tools \
	libdw-dev \
	libelf-dev \
	libiberty-dev \
	libslang2-dev \
	&& \
	rm -rf /var/lib/apt/lists/* && \
	pip3 install -U colorama cryptography gitpython pycodestyle pylint pylint-quotes PyYAML

