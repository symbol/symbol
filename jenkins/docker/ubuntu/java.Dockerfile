FROM ubuntu:24.04

# install dependencies (install tzdata first to prevent 'geographic area' prompt)
RUN apt-get update \
	&& apt-get install -y tzdata \
	&& apt-get install -y openjdk-11-jdk-headless git curl libssl-dev maven ca-certificates \
	&& update-ca-certificates

# install python
RUN apt-get install -y python3-pip python3-venv

# install shellcheck
RUN apt-get install -y shellcheck

# codecov uploader
RUN ARCH=$([ "$(uname -m)" = "x86_64" ] && echo "linux" || echo "aarch64") \
	&& curl -Os "https://uploader.codecov.io/latest/${ARCH}/codecov" \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu
USER ubuntu
WORKDIR /home/ubuntu
ENV PATH=$PATH:/home/ubuntu/.local/bin

# create a virtual environment
ENV VIRTUAL_ENV=/home/ubuntu/venv
RUN python3 -m venv $VIRTUAL_ENV
ENV PATH="$VIRTUAL_ENV/bin:$PATH"

# install gitlint
RUN pip install gitlint

WORKDIR /home/ubuntu
