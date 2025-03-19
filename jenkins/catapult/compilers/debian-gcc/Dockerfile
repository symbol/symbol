ARG FROM_IMAGE=debian:12
FROM ${FROM_IMAGE}

RUN apt-get update >/dev/null && \
	apt-get install -y \
	apt-utils \
	build-essential \
	gnupg2 \
	curl \
	xz-utils \
	wget \
	>/dev/null && \
	apt-get update >/dev/null && \
	apt-get install -y \
	gcc g++ \
	>/dev/null && \
	rm -rf /var/lib/apt/lists/*

# add ubuntu user (used by jenkins)
ARG HOME_DIR=/home/ubuntu
RUN groupadd -g 1000 ubuntu && \
	useradd -m -u 1000 -g ubuntu -d ${HOME_DIR} ubuntu
USER ubuntu
WORKDIR ${HOME_DIR}
ENV VIRTUAL_ENV=${HOME_DIR}/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"
