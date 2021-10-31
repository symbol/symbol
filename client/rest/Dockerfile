FROM ubuntu:20.04

RUN apt-get update && apt-get install -y curl \
    && curl -fsSL https://deb.nodesource.com/setup_12.x | bash - \
    && apt-get install -y nodejs \
    && node --version \
    && npm --version \
    && npm install -g yarn

WORKDIR /app
COPY . /app/catapult-rest
RUN cd catapult-rest && ./yarn_setup.sh
WORKDIR /app/catapult-rest/rest
