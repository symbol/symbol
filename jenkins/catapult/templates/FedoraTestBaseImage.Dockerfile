# image name required as ARG
ARG FROM_IMAGE='fedora:41'
ARG DEBIAN_FRONTEND=noninteractive

FROM ${FROM_IMAGE}
MAINTAINER Catapult Development Team
RUN dnf update --assumeyes && dnf install --assumeyes \
	bison \
	gdb \
	git \
	flex \
	python3 \
	python3-ply \
	python3-pip \
	ShellCheck \
	inotify-tools \
	elfutils-devel \
	elfutils-libelf-devel \
	slang-devel \
	&& \
	dnf clean all && \
	rm -rf /var/cache/yum && \
	pip3 install -U colorama conan cryptography gitpython pycodestyle pylint PyYAML

# add fedora user (used by jenkins)
ARG HOME_DIR=/home/fedora
RUN groupadd -g 1000 fedora && \
	useradd -m -u 1000 -g fedora -d ${HOME_DIR} fedora
USER fedora
WORKDIR ${HOME_DIR}
ENV VIRTUAL_ENV=${HOME_DIR}/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
