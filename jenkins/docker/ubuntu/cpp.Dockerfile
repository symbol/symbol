FROM symbolplatform/symbol-server-build-base:ubuntu-gcc-13-conan

# install shellcheck
RUN apt-get install -y shellcheck

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# mongodb 6.0
RUN apt-get install -y wget gnupg \
	&& wget -qO - https://www.mongodb.org/static/pgp/server-6.0.asc |  gpg --dearmor | tee /usr/share/keyrings/mongodb.gpg > /dev/null \
	&& echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb.gpg ] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/6.0 multiverse"  \
	| tee /etc/apt/sources.list.d/mongodb-org-6.0.list \
	&& apt-get update \
	&& apt-get install -y mongodb-org

RUN apt-get install -y python3-pip python3-venv

# add ubuntu user (used by jenkins)
RUN id -u "ubuntu" || useradd --uid 1000 -ms /bin/bash ubuntu

# Create the MongoDB data directory
RUN mkdir -p /data/db \
	&& chown -R ubuntu:ubuntu /data
USER ubuntu
WORKDIR /home/ubuntu

# installl all pip packages as ubuntu user
RUN pip install --upgrade colorama gitlint isort pycodestyle ply pylint PyYAML
