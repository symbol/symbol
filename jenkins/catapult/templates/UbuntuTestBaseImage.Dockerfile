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
	python3-pip \
	python3-venv \
	shellcheck \
	inotify-tools \
	libdw-dev \
	libelf-dev \
	libiberty-dev \
	libslang2-dev \
	&& \
	rm -rf /var/lib/apt/lists/*

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

RUN pip3 install -U colorama conan cryptography gitpython pycodestyle pylint ply PyYAML

USER root
RUN git clone https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git linux.git && \
	cd linux.git/tools/perf && \
	NO_LIBTRACEEVENT=1 make && \
	cp perf /usr/bin && \
	cd ../../.. && \
	rm -rf linux.git
