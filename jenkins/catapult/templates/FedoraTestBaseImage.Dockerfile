# image name required as ARG
ARG FROM_IMAGE=''
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
