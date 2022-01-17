FROM {{BASE_IMAGE}}
ENV DEBIAN_FRONTEND=noninteractive
MAINTAINER Catapult Development Team
RUN apt-get -y update && apt-get install -y \
    gdb \
    openssl \
    && \
    rm -rf /var/lib/apt/lists/*
