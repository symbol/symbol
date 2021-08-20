FROM node:12.18.1-alpine
RUN apk add --update\
 python \
 python3 \
 build-base \
 zeromq-dev \
 && rm -rf /var/cache/apk/*
WORKDIR /app
COPY . /app/catapult-rest
RUN cd catapult-rest \
 && ./yarn_setup.sh
RUN cd catapult-rest/rest
WORKDIR /app/catapult-rest/rest
