FROM symbolplatform/symbol-server-build-base:ubuntu-gcc-11-conan

# install shellcheck and gitlint
RUN apt-get install -y shellcheck
RUN pip install gitlint

# mongodb
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC 
RUN apt-get install -y wget gnupg \
	&& wget -qO- https://www.mongodb.org/static/pgp/server-5.0.asc | gpg --dearmor > /etc/apt/trusted.gpg.d/mongo.gpg \
	&& echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/5.0 multiverse" \
	| tee /etc/apt/sources.list.d/mongodb-org-5.0.list \

# mongodb requires ssl 1.1 but ubuntu 22.04 ships with ssl 3
	&& wget http://archive.ubuntu.com/ubuntu/pool/main/o/openssl/libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb \
	&& dpkg -i ./libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb \
	&& rm -i ./libssl1.1_1.1.1f-1ubuntu2.16_amd64.deb \
        && apt-get update \
	&& apt-get install -y mongodb-org

# add ubuntu user (used by jenkins)
RUN useradd --uid 1000 -ms /bin/bash ubuntu

# Create the MongoDB data directory
RUN mkdir -p /data/db \
	&& chown -R ubuntu:ubuntu /data

WORKDIR /home/ubuntu
