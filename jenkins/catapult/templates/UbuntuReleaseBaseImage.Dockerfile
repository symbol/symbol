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

# add catapult user (used by jenkins)
ARG HOME_DIR=/home/catapult
RUN groupadd -g 1000 catapult && \
	useradd -m -u 1000 -g catapult -d ${HOME_DIR} catapult
USER catapult
WORKDIR ${HOME_DIR}
ENV VIRTUAL_ENV=${HOME_DIR}/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
