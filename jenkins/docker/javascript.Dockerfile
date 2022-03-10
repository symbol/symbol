FROM ubuntu:20.04

# install tzdata first to prevent 'geographic area' prompt
RUN apt-get update >/dev/null \
	&& apt-get install -y tzdata \
	&& apt-get install -y git curl

# mongodb
RUN apt-get install -y wget gnupg \
	&& wget -qO - https://www.mongodb.org/static/pgp/server-5.0.asc | apt-key add - \
	&& echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/5.0 multiverse" \
	| tee /etc/apt/sources.list.d/mongodb-org-5.0.list \
	&& apt-get update \
	&& apt-get install -y mongodb-org

# nodejs
RUN curl -fsSL https://deb.nodesource.com/setup_16.x | bash - \
	&& apt-get install -y nodejs

# rest dependencies
RUN apt-get install -y make gcc g++

# install python
RUN apt-get install -y python3-pip

# install shellcheck and gitlint
RUN apt-get install -y shellcheck \
	&& pip install gitlint

# codecov uploader
RUN curl -Os https://uploader.codecov.io/latest/linux/codecov \
	&& chmod +x codecov \
	&& mv codecov /usr/local/bin

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu

# Create the MongoDB data directory
RUN mkdir -p /data/db \
	&& chown -R ubuntu:ubuntu /data

WORKDIR /home/ubuntu
